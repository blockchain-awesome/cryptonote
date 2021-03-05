#!/usr/bin/env bash
rm build -rf
mkdir -p build

cd build
cmake .. || exit 1
make || exit 1
