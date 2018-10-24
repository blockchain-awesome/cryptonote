#!/usr/bin/env bash
echo "$1"
if [ -z "$1" ]; then
echo "please input a pattern"
else

find ./src/ -type f -exec sed -i "/$1/d" {} +
find ./include/ -type f -exec sed -i "/$1/d" {} +
find ./tests/ -type f -exec sed -i "/$1/d" {} +
fi


