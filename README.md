VIG COIN是基于[CryptoNote](https://cryptonote.org) 加密币协议的币。

它旨在在当前加密币的基础上：

1. 继续完善加密币或者周边（包括去中心化，加快交易速度等）
2. 推动加密币的实际使用场景（包括尝试构建去中心交易所等）


## 参与

VIG COIN是一个开源，社区化的项目。非常欢迎各类开源贡献者的参与和支持。
同时VIG COIN也提供了回报机制鼓励更多人的参与。

详细可以参考：[wiki](https://github.com/vigcoin/wiki)

## 算法

目前基于cryptonight，但是基于去中心化，反对ASIC的考虑，未来算法会不断的修改。

## 编译

推荐 Ubuntu 16.04 平台

## ComplexWallet

ComplexWallet是VIG COIN特有的服务，它是为去中心化交易做准备的钱包服务。

### On *nix

Dependencies: GCC 4.7.3 or later, CMake 2.8.6 or later, and Boost 1.55.

You may download them from:

* http://gcc.gnu.org/
* http://www.cmake.org/
* http://www.boost.org/
* Alternatively, it may be possible to install them using a package manager.

To build, change to a directory where this file is located, and run `make`. The resulting executables can be found in `build/release/src`.

**Advanced options:**

* Parallel build: run `make -j<number of threads>` instead of `make`.
* Debug build: run `make build-debug`.
* Test suite: run `make test-release` to run tests in addition to building. Running `make test-debug` will do the same to the debug version.
* Building with Clang: it may be possible to use Clang instead of GCC, but this may not work everywhere. To build, run `export CC=clang CXX=clang++` before running `make`.

### On Windows
Dependencies: MSVC 2013 or later, CMake 2.8.6 or later, and Boost 1.55. You may download them from:

* http://www.microsoft.com/
* http://www.cmake.org/
* http://www.boost.org/

To build, change to a directory where this file is located, and run theas commands: 
```
mkdir build
cd build
cmake -G "Visual Studio 12 Win64" ..
```

And then do Build.
Good luck!