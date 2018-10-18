#!/usr/bin/env bash
echo "$1"
echo "$2"

if [ -z "$1" ] || [ -z "$2" ]; then
echo "please input both replacee and replacer"
else

find ./src/ -type f -exec sed -i "s/$1/$2/g" {} +
find ./include/ -type f -exec sed -i "s/$1/$2/g" {} +
find ./tests/ -type f -exec sed -i "s/$1/$2/g" {} +
fi


