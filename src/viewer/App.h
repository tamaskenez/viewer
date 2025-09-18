#include "util/SDLApp.h"

#include <memory>

// Create the main application object.
std::unique_ptr<SDLApp> make_app(int argc, char** argv);
