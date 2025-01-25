#!/usr/bin/bash

rm -rf ./build
mkdir build

SCRIPT_DIR=$(dirname ${BASH_SOURCE[0]})
BUILD_DIR="$SCRIPT_DIR/build"
SERVER_SOURCE_DIR="$SCRIPT_DIR/server"
CLIENT_SOURCE_DIR="$SCRIPT_DIR/client"
NET_SOURCE_DIR="$SCRIPT_DIR/net"

gcc -I$SCRIPT_DIR -o "$BUILD_DIR/server" "$SERVER_SOURCE_DIR/main.c"
gcc -I$SCRIPT_DIR -o "$BUILD_DIR/client" "$CLIENT_SOURCE_DIR/main.c"
