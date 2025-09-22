#include "http.h"

#include "util/sdl_util.h"

#include <SDL3/SDL_timer.h>
#include <meadow/cppext.h>

#include <print>

#ifdef __EMSCRIPTEN__
  #include <emscripten/fetch.h>
#endif

namespace
{
struct PendingRequest {
    std::string body;
    HttpRequestCompletionHandlerFn completion_handler;
};
} // namespace

#ifdef __EMSCRIPTEN__
namespace
{
void onsuccess(emscripten_fetch_t* fetch)
{
    std::println(
      "Finished downloading {} bytes from URL {}. Status: {} ({})\n",
      fetch->numBytes,
      fetch->url,
      fetch->statusText,
      fetch->status
    );
    // The data is now available at fetch->data[0] through fetch->data[fetch->numBytes-1];
    auto* pending_request = reinterpret_cast<PendingRequest*>(fetch->userData);
    std::string response;
    if (fetch->data) {
        response = std::string(fetch->data, fetch->numBytes);
    }
    pending_request->completion_handler(fetch->status, fetch->statusText, MOVE(response));
    delete pending_request;
    emscripten_fetch_close(fetch); // Free data associated with the fetch.
}

void onerror(emscripten_fetch_t* fetch)
{
    std::println("Downloading {} failed, status: {} ({}).\n", fetch->url, fetch->statusText, fetch->status);
    auto* pending_request = reinterpret_cast<PendingRequest*>(fetch->userData);
    pending_request->completion_handler(fetch->status, fetch->statusText, "<failed-request-no-response>");
    delete pending_request;
    emscripten_fetch_close(fetch); // Also free data on failure.
}
} // namespace

void send_http_post_request(
  const char* url, const char* content_type, std::string body, HttpRequestCompletionHandlerFn completion_handler
)
{
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);

    auto* pending_request = new PendingRequest{.body = MOVE(body), .completion_handler = MOVE(completion_handler)};

    strcpy(attr.requestMethod, "POST");
    attr.userData = pending_request;
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;

    attr.onsuccess = onsuccess;
    attr.onerror = onerror;

    // Set headers (application/json)
    const char* headers[] = {"Content-Type", content_type, nullptr};
    attr.requestHeaders = headers;

    attr.requestData = pending_request->body.data();
    attr.requestDataSize = pending_request->body.size();

    std::println("emscripten_fetch(..., \"{}\")", url);
    std::println("attr.requestMethod = {}", attr.requestMethod);
    std::println("content_type = {}", content_type);
    std::println("attr.requestDataSize = {}", attr.requestDataSize);
    emscripten_fetch(&attr, url);
}
#else
namespace
{
uint32_t timer_callback(void* userdata, SDL_TimerID /*timerID*/, Uint32 /*interval*/)
{
    auto* pending_request = reinterpret_cast<PendingRequest*>(userdata);
    pending_request->completion_handler(123, "Fake HTTP request done.", "This is the response.");
    delete pending_request;
    return 0;
}
} // namespace

// Simulate request on desktop.
void send_http_post_request(
  const char* /*url*/, const char* /*content_type*/, std::string body, HttpRequestCompletionHandlerFn completion_handler
)
{
    auto* pending_request = new PendingRequest{.body = MOVE(body), .completion_handler = MOVE(completion_handler)};
    CHECK_SDL(SDL_AddTimer(1000, &timer_callback, pending_request));
}
#endif
