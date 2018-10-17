if [ ! -d "build" ]; then
    mkdir -p build
fi
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_GCOV=1 || exit 1
make || exit 1
env CTEST_OUTPUT_ON_FAILURE=1 make test || exit 1
# lcov --capture --directory src --remove '/usr/include/*' './include/*' --output-file coverage.info
# lcov --remove coverage.info '/usr/include/*' './include/*' --output-file coverage1.info
lcov --capture --directory src --output-file temp.info || exit 1
lcov --remove temp.info "/usr/include/*" "./include/*" > coverage.info || exit 1
lcov --list coverage.info || exit 1
genhtml coverage.info --output-directory html || exit 1

# codecov --token 2a2cf36f-81d6-43b9-abba-d5955cb0cafe --root src

bash <(curl -s https://codecov.io/bash) -t 2a2cf36f-81d6-43b9-abba-d5955cb0cafe || exit 1

xdg-open ./html/index.html
