#! /bin/bash

cd "$(dirname "$0")"

if [ -d "build" ]; then
    cd build && rm -rf *
    echo "dsfd"
else
    mkdir build && cd build
    echo "hiihi"
fi
cmake ..
make