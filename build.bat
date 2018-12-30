rmdir build /S /Q
mkdir build
cd build
cmake -G "Visual Studio 15 2017 Win64" ..
msbuild vigcoin.sln
ctest --verbose
cd ..
