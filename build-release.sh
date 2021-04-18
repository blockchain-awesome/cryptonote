#!/usr/bin/env bash
mkdir -p build

cd build
cmake .. -DENABLE_GCOV=1 || exit 1
make || exit 1
make test || exit 1
