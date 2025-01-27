#!/usr/bin/bash

rm -rf ./build
mkdir build

SCRIPT_DIR=$(dirname ${BASH_SOURCE[0]})
BUILD_DIR="$SCRIPT_DIR/build"
SRC_DIR="$SCRIPT_DIR/src"
SERVER_SOURCE_DIR="$SRC_DIR/server"
CLIENT_SOURCE_DIR="$SRC_DIR/client"

gcc -I$SRC_DIR -o "$BUILD_DIR/server" "$SERVER_SOURCE_DIR/main.c"
gcc -I$SRC_DIR -o "$BUILD_DIR/client" "$CLIENT_SOURCE_DIR/main.c"
