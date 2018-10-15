if [ ! -d "build" ]; then
    mkdir -p build
fi
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_GCOV=1
make
env CTEST_OUTPUT_ON_FAILURE=1 make test
lcov --capture --directory src --remove '/usr/include/' './include' --output-file coverage.info
genhtml coverage.info --output-directory html

xdg-open ./html/index.html
