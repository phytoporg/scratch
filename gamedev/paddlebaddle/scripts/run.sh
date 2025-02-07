#!/usr/bin/bash

if [[ -z "${BUILD_EMSCRIPTEN}" ]]; then
    ./build/paddlebaddle $(pwd)/assets &
else 
    # Not supported yet
    exit -1
    # emrun ./embuild/index.html
fi
