#!/usr/bin/bash

if [[ -z "${BUILD_EMSCRIPTEN}" ]]; then
    rm -rf build
    mkdir build
    gcc -o build/paddlebaddle src/pb.c `sdl2-config --cflags --libs` -lm # -lSDL2_mixer -lSDL2_ttf
else
    # Not supported yet
    exit -1

    # rm -rf embuild
    # mkdir embuild
    # emcc src/pb.c --emrun -s USE_SDL=2 -s USE_SDL_TTF=2 -s USE_FREETYPE=1 -s USE_SDL_MIXER=2 -s TOTAL_MEMORY=1024MB --preload-file ./assets -o ./embuild/index.html

    # # The default emscripten template is uggers
    # if [ $? -eq 0 ]; then
    #     cp ./wasm/index.html ./embuild
    # fi
fi

