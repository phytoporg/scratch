#!/usr/bin/bash

if [[ -z "${BUILD_EMSCRIPTEN}" ]]; then
    ./build/defren ./assets
else 
    >&2 echo emscripten build not yet supported
    # emrun ./embuild/index.html
fi