#pragma once

#include <string>

namespace tgping {
class TelegramClient {
public:
  TelegramClient(std::string auth_key);
  ~TelegramClient();

  void send_message(const std::string &chat_id,
                    const std::string &messgae) const;

private:
  std::string _auth_key;
};
} // namespace tgping
