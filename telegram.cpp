#include <iostream>
#include <sstream>
#include <utility>

#include <curl/curl.h>

#include "telegram.h"

namespace tgping {
namespace {
std::string build_send_message_json(const std::string &chat_id,
                                    const std::string &message) {
  std::stringstream json;
  json << "{";
  json << "\"chat_id\":\"" << chat_id << "\"";
  json << ",";
  json << "\"text\":\"" << message << "\"";
  json << "}";
  return json.str();
}

bool https_post(const std::string &url, const std::string &body) {
  struct curl_slist *headers = nullptr;
  headers = curl_slist_append(headers, "Content-Type: application/json");

  auto *curl = curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_POST, 1L);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
  const auto result = curl_easy_perform(curl);

  curl_easy_cleanup(curl);
  curl_slist_free_all(headers);

  return result == CURLE_OK;
}
} // namespace

TelegramClient::TelegramClient(std::string auth_key)
    : _auth_key(std::move(auth_key)) {
  curl_global_init(CURL_GLOBAL_ALL);
}

TelegramClient::~TelegramClient() { curl_global_cleanup(); }

void TelegramClient::send_message(const std::string &chat_id,
                                  const std::string &message) const {
  using namespace std::string_literals;
  const std::string url =
      "https://api.telegram.org/bot"s + _auth_key + "/sendMessage"s;
  const std::string body = build_send_message_json(chat_id, message);
  https_post(url, body);
}
} // namespace tgping
