#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <thread>
#include <iomanip>

#include "telegram.h"
#include "config.h"

using namespace tgping;

namespace {
std::string do_ping(const int ipv, const std::string &host) {
  std::stringstream cmd_ss;
  cmd_ss << "/usr/bin/ping ";
  cmd_ss << "-" << ipv << " "; // ipv4 or ipv6
  cmd_ss << "-q ";             // quiet, only show result
  cmd_ss << "-c 1 ";           // only do one ping
  cmd_ss << host;
  const std::string cmd = cmd_ss.str();

  auto *fp = popen(cmd.c_str(), "r");
  assert(fp != nullptr || "could not invoke ping command");

  std::stringstream out_ss;
  char *line = nullptr;
  std::size_t line_len = 0;
  while ((line_len = getline(&line, &line_len, fp)) != -1) {
    out_ss << line;
  }

  free(line);
  pclose(fp);

  return out_ss.str();
}

// -2: unexpected format, could not parse
// -1: packet dropped
// 1: ok
int parse_ping_output(const std::string &output) {

  const std::string str_transmitted = "packets transmitted, ";
  const std::string str_received = "received, ";

  const std::size_t pos_transmitted = output.find(str_transmitted);
  if (pos_transmitted == std::string::npos) {
    return -2;
  }

  const std::size_t pos_received = output.find(str_received, pos_transmitted);
  if (pos_received == std::string::npos) {
    return -2;
  }

  const std::string out_received = output.substr(
      pos_transmitted + str_transmitted.length(),
      pos_received - pos_transmitted - str_transmitted.length() - 1);

  if (out_received == "0") {
    return -1;
  } else if (out_received == "1") {
    return 1;
  } else {
    return -2;
  }
}

double collect_statistics(const int ipv, const std::string &host,
                          const int interval, const int minutes) {
  using namespace std::chrono_literals;

  int successful_pings = 0;
  int dropped_pings = 0;

  const int iterations = minutes * 60 / interval;
  for (int it = 0; it < iterations; ++it) {
    const auto output = do_ping(ipv, host);
    const auto ans = parse_ping_output(output);

    std::cout << output << "----> " << ans << std::endl;
    if (ans == -2) {
      std::cout << "Could not parse ping output:\n";
      std::cout << "--------------------\n";
      std::cout << output << "\n";
      std::cout << "--------------------\n";
      std::cout << std::endl;

      ++dropped_pings;
    } else if (ans == -1) {
      ++dropped_pings;
    } else {
      ++successful_pings;
    }

    std::this_thread::sleep_for(1s * interval);
  }

  return 1.0 * dropped_pings / (successful_pings + dropped_pings);
}
} // namespace

int main() {
  TelegramClient client(AUTH_KEY);

  while (true) {
    const auto ans = collect_statistics(4, HOST, INTERVAL, DURATION * 60);
    std::stringstream message;
    message << "Dropped " << std::setprecision(2) << 100.0 * ans << "% during the last hour";
    client.send_message(CHAT_ID, message.str());
  }

  return 0;
}
