# VIEWER

Simple 3D viewer app, for web and native.

## Building for the web

Install the latest emscripten SDK: https://emscripten.org/docs/getting_started/downloads.html

```
emcmake cmake -S . -B bw -DCMAKE_BUILD_TYPE=Debug 
cmake --build bw --target viewer -j
```

## Building for native

Configure cmake: `cmake -S . -B b -GXcode`, then open Xcode (or other IDE) and run `viewer`.
