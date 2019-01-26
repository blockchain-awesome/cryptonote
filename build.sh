#!/usr/bin/env bash
if [ ! -d "build" ]; then
    mkdir -p build
fi
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_GCOV=1 || exit 1
make || exit 1
