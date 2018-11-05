#!/usr/bin/env bash
echo "$1"
echo "$2"

if [ -z "$1" ] || [ -z "$2" ]; then
echo "please input both replacee and replacer"
else
 
for dir in "./src/" "./include/" "./tests/"; do
echo "1";
find $dir -type f -exec sed -i "s/\([^[:alpha:]]\+\)$1\([^[:alpha:]]\+\)/\1$2\2/g" {} +
echo "2";
find $dir -type f -exec sed -i "s/\([^[:alpha:]]\+\)$1\($\)/\1$2\2/g" {} +
echo "3";
find $dir -type f -exec sed -i "s/^$1\([^[:alpha:]]\+\)/$2\1/g" {} +
done
fi


