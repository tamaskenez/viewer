# VIEWER

Simple 3D viewer app, for web and native.

You can try out the deployed web app here: https://tamaskenez.github.io/viewer/

## Features

- Simple 3D viewer with orbit/pan/zoom navigation
- Using the `assimp` library to import 3D files, for now the COLLADA, OBJ and STL importers are enabled.
- 5 built-in models, downloaded from https://mecabricks.com
- Customize light direction and background color
- JSON file containing customization sent to https://echo.free.beeceptor.com (which does reply).
- User models can be pasted into the browser window (small files only).
- Colorspace-correct lighting (calculations in linear color space)

## About the implementation

The application is built with the SDL3 + ImGUI + Emscripten tech stack in C++.

## Known bugs:

Due to the limitation of the third-party component which intercepts text pasted into the browser window, only small files can be pasted. Please try the paste_test*.txt files in the root of this repository.

## Building the app

Don't forget checking out the submodules.

### Building the web app

Install the latest emscripten SDK: https://emscripten.org/docs/getting_started/downloads.html, then:

```
./build_em.sh # Configures cmake, builds and runs it with a local server using "emrun"
```

Note: `./build_em.sh` tries launching the Chrome browser, use `emrun ./bw/src/viewer/viewer.html` to run in the default browser.

## Building the native app

Configure cmake: `cmake -S . -B b -GXcode`, then open Xcode (or other IDE) and run `viewer`.
