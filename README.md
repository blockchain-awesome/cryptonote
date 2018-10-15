
## VIG

[![Coverity Scan Build Status](https://scan.coverity.com/projects/16947/badge.svg)](https://scan.coverity.com/projects/vigcoin-coin)
[![codecov](https://codecov.io/gh/vigcoin/coin/branch/master/graph/badge.svg)](https://codecov.io/gh/vigcoin/coin)
[![CircleCI](https://circleci.com/gh/vigcoin/coin.svg?style=svg)](https://circleci.com/gh/vigcoin/coin)
[![LICENSE](https://img.shields.io/badge/license-mit-blue.svg?longCache=true)](https://opensource.org/licenses/MIT)


[![Gitter Chat](https://img.shields.io/gitter/room/badges/shields.svg?logo=gitter)](https://gitter.im/vigcoin/development)
[![Discord](https://img.shields.io/badge/discord-join%20chat-blue.svg?logo=discord)](https://discord.gg/dWWDFKb)
[![QQ group](https://img.shields.io/badge/style-group%20chat-green.svg?longCache=true&label=qq&colorA=ff69b4&colorB=gray&logo=data:image/svg+xml;base64,PHN2ZyBhcmlhLWxhYmVsbGVkYnk9InNpbXBsZWljb25zLXRlbmNlbnRxcS1pY29uIiByb2xlPSJpbWciIHZpZXdCb3g9IjAgMCAyNCAyNCIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj48dGl0bGUgaWQ9InNpbXBsZWljb25zLXRlbmNlbnRxcS1pY29uIj5UZW5jZW50IFFRIGljb248L3RpdGxlPjxwYXRoIGQ9Ik0yMC4wOSAxMS43MDljLS4wNjQtLjkwNy0uMjM3LTEuOTk1LS43MDMtMi42MzguNDUxLTUuNzM1LTMuNDUtOS4xMjktNy4zODYtOS4wNy0zLjkzNi4wNjQtNy44MzQgMy4zMzUtNy4zODMgOS4wNy0uNDY2LjY0My0uNjQyIDEuNzI1LS43IDIuNjM4LTEuMTU1IDEuNzY5LTIuNDI5IDQuNDctMS4wMDkgNy4wNzQuNzIzLS4zMTQgMS4zMDUtLjk5OCAxLjQ5NS0xLjg0NS41NSAxLjM1IDEuMzU0IDIuNjMgMi40MzggMy42MTUtMS41NTMuMzA0LTIuNjU1IDEuMDM2LTIuNjE4IDEuODMzLjA0NS45ODMgMS44MDUgMS43MDQgMy45MjYgMS42MDQgMS43NjYtLjA4NSAzLjIyNS0uNzA1IDMuNjQxLTEuNDgxLjA0NS4wMDYuMzc5LjAwNi40MjQgMCAuNDE0Ljc3OCAxLjg3NSAxLjM5OSAzLjY0MSAxLjQ4MSAyLjEyMS4wOTcgMy44ODEtLjYyMSAzLjkyNS0xLjYwNC4wMzctLjgtMS4wNjQtMS41MjktMi42MTUtMS44MyAxLjA4NS0uOTkgMS44OTEtMi4yNjYgMi40MzctMy42MTUuMTkuODQ3Ljc3MiAxLjUzIDEuNDk0IDEuODQ1IDEuNDE1LTIuNjA0LjE0MS01LjMwNS0xLjAxMS03LjA3NGgtLjAwM3YtLjAwM2guMDA3ek0xMy45ODMgMy43N2MuNzgxLjAyMyAxLjM4Ny45NzYgMS4zNTIgMi4xMjQtLjAzOSAxLjE0OC0uNzAxIDIuMDU5LTEuNDggMi4wMzMtLjc3OS0uMDI3LTEuMzc5LS45NzMtMS4zNDUtMi4xMjUuMDM0LTEuMTQ4LjY5OS0yLjA1NSAxLjQ3My0yLjAzMnptLTQuMDgyLS4xM2MuODA0LS4wMjkgMS40ODQuOTEyIDEuNTI0IDIuMDk4LjAzNyAxLjE4Ni0uNTg1IDIuMTY4LTEuMzg4IDIuMTktLjgwMy4wMy0xLjQ4NC0uOTA5LTEuNTIzLTIuMDk2LS4wMzYtMS4xODUuNTg0LTIuMTY5IDEuMzg2LTIuMTlsLjAwMS0uMDAyek03LjQxNSA5LjI0N2MuMTA0LS40NjUgMi44NzUtMS4wMjkgNC41ODYtMS4wMzUgMS43MTUtLjAxMSA0LjQ4Mi41NzMgNC41OSAxLjAzNS4wNjkuMzQxLTEuNDQ5IDEuNTE4LTIuNDA1IDEuODk5LS45MjMuMzU5LTEuMTMuODczLTIuMTgzLjg3My0xLjA1MSAwLTEuMjYtLjUxMS0yLjE4LS44NzMtLjk1OS0uMzgyLTIuNDc4LTEuNTYtMi40MS0xLjg5OWguMDAyem00LjU4NiAxMi42ODNjLTMuNjUyLS4wMTItNy4yNTktMi44OTUtNS42NzQtOC41ODcuMjE5LjEyMS40NTUuMjI5LjcwMi4zMzgtLjA1Mi43MTUtLjIwNiAzLjAzLS4xNCAzLjM5Ny4wNzUuNDQzIDIuNDMuNzc0IDIuNjQxLjYxOC4xODYtLjEzNS4xNjQtMi40MDguMTM5LTMuMjE5Ljc1My4xNDMgMS41NDIuMjIxIDIuMzMyLjIyMSAyLjEyLjAwMiA0LjIzLS41OCA1LjY3NS0xLjM1MSAxLjU5IDUuNjg4LTIuMDE5IDguNTk1LTUuNjczIDguNTgzaC0uMDAyeiIvPjxwYXRoIGQ9Ik0xMC4zNzcgNC43NzRjLS4zMzkuMDEyLS42LjQ0Mi0uNTg1Ljk2NS4wMTYuNTI0LjMwMy45MzguNjQxLjkzLjMzNy0uMDE2LjYtLjQ0NC41ODQtLjk2Ny0uMDE0LS41Mi0uMzAzLS45MzYtLjY0LS45MjZ2LS4wMDJ6bS4xNzYgMS4yNDZjLS4xMjYuMDA2LS4yMzUtLjE2MS0uMjQtLjM3NS0uMDA5LS4yMTQuMDg3LS4zODguMjE1LS4zOS4xMjctLjAwNi4yMzUuMTY0LjI0Mi4zNzUuMDA3LjIwOS0uMDg5LjM4Ny0uMjE3LjM5em0zLjg3OS4yNzRoLS4wMDljLS4wODQtLjAwNC0uMTUyLS4wNzUtLjE0OC0uMTU4LjAwOC0uMTQ4LS4wMTktLjU2NC0uMjA3LS43NjQtLjA3LS4wNzYtLjE1Ni0uMTA5LS4yNjgtLjEwOS0uNTE0IDAtLjUyNS43NTQtLjUyNy43ODUgMCAuMDktLjA2NS4xNTctLjE1Ni4xNTctLjA5MiAwLS4xNS0uMDc1LS4xNS0uMTU5IDAtLjM3OS4xNzctMS4wOTUuODM4LTEuMDk1LjE5NCAwIC4zNTUuMDcxLjQ4OC4yMDYuMzE4LjMzNC4yOTUuOTY3LjI5My45OS0uMDAxLjA3OS0uMDY5LjE0My0uMTU0LjE0N3pNMTIuMDAxIDExLjVjLjE5OC4wMDYuMzAxLS4wMTIuNDQ3LS4wMzcuMzc1LS4wNzMuNzU2LS40MjkgMS4xMTYtLjYzLjc3NS0uNDguOTE3LS4zOTEgMi4xNDYtMS40OC0uODI4LjY3LTEuNjA1IDEuMDI3LTIuNDEyIDEuMjY0LS4yMi4wNjQtLjc0NC4xMDUtMS4yOTUuMTA4LS41NTEuMDA5LTEuMDc1LS4wNDUtMS4yOTUtLjEwOC0uODA3LS4yMzctMS41ODQtLjU5MS0yLjQwOC0xLjI2NiAxLjIyNiAxLjA5IDEuMzcxLjk5OSAyLjE0NSAxLjQ3OS4zNi4yMDMuNzQxLjU2MyAxLjExNi42MzQuMTQxLjAyNS4yNDcuMDMxLjQ0LjAzNnoiLz48L3N2Zz4=)](http://shang.qq.com/wpa/qunwpa?idkey=aa5b2ba04d44820536a6aad31e3d59fb3e43fbed4cbdf1215b204ed61f22036e)


Copyright (c) 2018- The VIG Project.  
Copyright (c) 2018- The Monero Project.  
Copyright (c) 2012-2013 The Cryptonote developers.  



## Introduction

VIG Coin is a coin based on the cryptonote cryptocurrency protocol.

It's goal is to implement a coin with:

1. Fast exchanging speed
2. Anti-ASIC awareness.
3. Decentralized exchange support
4. A light wallet api
5. Node with one Docker command
6. Offlinable Web based decentralized wallet


## 参与

VIG COIN是一个开源，社区化的项目。非常欢迎各类开源贡献者的参与和支持。
同时VIG COIN也提供了回报机制鼓励更多人的参与。

详细可以参考：[wiki](https://github.com/vigcoin/wiki)

## 算法

目前基于cryptonight，但是基于去中心化，反对ASIC的考虑，未来算法会不断的修改。

## 编译

推荐 Ubuntu 16.04 平台

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

## License

MIT

[travis-image]: https://travis-ci.org/vigcoin/coin.svg?branch=master
[travis-url]: https://travis-ci.org/vigcoin/coin
[coveralls-image]: https://coveralls.io/repos/github/vigcoin/coin/badge.svg?branch=master
[coveralls-url]: https://coveralls.io/github/vigcoin/coin?branch=master