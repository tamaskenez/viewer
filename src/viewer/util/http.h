#pragma once

#include <functional>
#include <string>
#include <string_view>

using HttpRequestCompletionHandlerFn = std::function<void(int status, std::string status_text, std::string response)>;

void send_http_post_request(
  const char* url, const char* content_type, std::string body, HttpRequestCompletionHandlerFn completion_handler
);
