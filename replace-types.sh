#!/usr/bin/env bash
echo "$1"
echo "$2"

if [ -z "$1" ] || [ -z "$2" ]; then
echo "please input both replacee and replacer"
else
 
for dir in "./src/" "./include/" "./tests/"; do
find $dir -type f -exec sed -i "s/ $1 / $2 /g" {} +
# find $dir -type f -exec sed -i "s/<$1>/<$2>/g" {} +
# find $dir -type f -exec sed -i "s/($1)/($2)/g" {} +
# find $dir -type f -exec sed -i "s/($1 /($2 /g" {} +
# find $dir -type f -exec sed -i "s/($1\&/($2\&/g" {} +
# find $dir -type f -exec sed -i "s/ $1\&/ $2\&/g" {} +
# find $dir -type f -exec sed -i "s/ $1)/ $2)/g" {} +
# find $dir -type f -exec sed -i "s/ $1\*/ $2\*/g" {} +
# find $dir -type f -exec sed -i "s/<$1,/<$2,/g" {} +
# find $dir -type f -exec sed -i "s/, $1>/, $2>/g" {} +
# find $dir -type f -exec sed -i "s/^$1 /$2 /g" {} +
# find $dir -type f -exec sed -i "s/<$1\*>/<$2\*> /g" {} +
# find $dir -type f -exec sed -i "s/ $1,/ $2,/g" {} +
# find $dir -type f -exec sed -i "s/<$1\&/<$2\&/g" {} +
# find $dir -type f -exec sed -i "s/ $1()/ $2()" {} +

done
fi


