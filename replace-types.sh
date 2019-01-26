#!/usr/bin/env bash
echo "$1"
echo "$2"

function replaceDir() {

replacee=$1
replacer=$2;
shift
shift
dirs=("$@")
echo ${dirs[*]};

if [ -z "$replacee" ] || [ -z "$replacer" ]; then
echo "please input both replacee and replacer"
else
 
for dir in "${dirs[@]}"; do
echo "Inside Dir $dir";
echo "1";
find $dir -type f -exec sed -i "s/\([^[:alpha:]]\+\)$replacee\([^[:alpha:]]\+\)/\1$replacer\2/g" {} +
echo "2";
find $dir -type f -exec sed -i "s/\([^[:alpha:]]\+\)$replacee\($\)/\1$replacer\2/g" {} +
echo "3";
find $dir -type f -exec sed -i "s/^$replacee\([^[:alpha:]]\+\)/$replacer\1/g" {} +
done
fi
}

dirs=(./src/ ./include/ ./tests/);

echo ${dirs[*]};

replaceDir $1 $2 "${dirs[@]}"




