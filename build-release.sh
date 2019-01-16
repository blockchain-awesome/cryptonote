rm build -rf
mkdir -p build

cd build
cmake .. -DENABLE_GCOV=1 || exit 1
make || exit 1
