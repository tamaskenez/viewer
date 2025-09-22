#include "util/SDLApp.h"

#include <memory>

inline constexpr auto k_http_endpoint = "https://echo.free.beeceptor.com/";

// Create the main application object.
std::unique_ptr<SDLApp> make_app(int argc, char** argv);
