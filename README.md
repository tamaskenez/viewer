# VIEWER

Simple 3D viewer app, for web and native.

You can try out the deployed web app here: https://tamaskenez.github.io/viewer/

## Features



## Building for the web

Install the latest emscripten SDK: https://emscripten.org/docs/getting_started/downloads.html

```
./build_em.sh # Configures cmake, builds and runs it with a local server using "emrun"
```

Note: `./build_em.sh` tries launching the Chrome browser, use `emrun ./bw/src/viewer/viewer.html` to run in the default browser.

## Building for native

Configure cmake: `cmake -S . -B b -GXcode`, then open Xcode (or other IDE) and run `viewer`.
