#! /bin/bash

cd "$(dirname "$0")"

if [ -d "build" ]; then
    cd build && rm -rf *
else
    mkdir build && cd build
fi
cmake ..
make