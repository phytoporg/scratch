#!/usr/bin/bash

if [[ -z "${BUILD_EMSCRIPTEN}" ]]; then
    rm -rf build
    mkdir build
    gcc -o build/defren src/main.c `sdl2-config --cflags --libs`  -lGL
else
    >&2 echo emscripten build not yet supported
    # rm -rf embuild
    # mkdir embuild
    # emcc src/lil-tetris.c --emrun -s USE_SDL=2 -s TOTAL_MEMORY=1024MB --preload-file ./assets -o ./embuild/index.html
fi

