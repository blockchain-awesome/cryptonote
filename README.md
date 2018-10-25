
# The VIG Project

## Building a decentralized, easy to use, high available coin for everyone

[![Coverity Scan Build Status](https://scan.coverity.com/projects/16947/badge.svg)](https://scan.coverity.com/projects/vigcoin-coin) [![LICENSE](https://img.shields.io/badge/license-GPL%20v3-blue.svg?longCache=true)](https://opensource.org/licenses/GPL-3.0)

[![Gitter Chat](https://img.shields.io/gitter/room/badges/shields.svg?logo=gitter)](https://gitter.im/vigcoin/development)
[![Discord](https://img.shields.io/badge/discord-join%20chat-blue.svg?logo=discord)](https://discord.gg/dWWDFKb)
[![QQ group](https://img.shields.io/badge/style-group%20chat-green.svg?longCache=true&label=qq&colorB=blue&logo=data:image/svg+xml;base64,PHN2ZyBoZWlnaHQ9IjI4OCIgdmlld0JveD0iMCAwIDEyMSAxNDQiIHdpZHRoPSIyNDIiIHhtbG5zPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwL3N2ZyI+PHBhdGggZD0iTTYxIDE0MmMtMTMgMC0yNS00LTMyLTEwbC0xMiA1Yy0yIDItMiA0LTEgNSAyIDMgMzUgMiA0NSAxem0wIDBNNjEgMTQyYzEyIDAgMjQtNCAzMS0xMGwxMiA1YzIgMiAyIDQgMSA1LTIgMy0zNSAyLTQ0IDF6bTAgMCIgZmlsbD0iI2ZhYWIwNyIvPjxwYXRoIGQ9Ik02MSA2N2MyMCAwIDM3LTQgNDItNmwzLTF2LTVjMC0yNy0xMy01NS00Ni01NS0zMiAwLTQ1IDI4LTQ1IDU1djVsMiAxYzUgMiAyMiA2IDQzIDZ6bTAgME0xMTcgOTBsLTUtMTNoLTJjLTE1IDQtMzUgNy00OSA3aC0xYy0xNCAwLTMzLTMtNDktN0g5TDQgOTBjLTYgMjAtNCAyOC0yIDI4IDMgMSAxMi0xNSAxMi0xNSAwIDE2IDE0IDQwIDQ2IDQwaDFjMzIgMCA0Ni0yNCA0Ni00MCAwIDAgOSAxNiAxMiAxNSAyIDAgNC04LTItMjgiLz48cGF0aCBkPSJNNDkgNDFjLTQgMC04LTUtOC0xMSAwLTcgMy0xMiA3LTEyIDUgMCA4IDUgOSAxMSAwIDYtNCAxMi04IDEyTTgwIDMwYzAgNi00IDExLTggMTFzLTgtNi04LTEyYzEtNiA0LTExIDktMTEgNCAwIDcgNSA3IDEyIiBmaWxsPSIjZmZmIi8+PHBhdGggZD0iTTg4IDUwYy0xLTMtMTMtNi0yNy02aC0xYy0xNCAwLTI2IDMtMjcgNnYxYzEgMSAxNCA4IDI3IDhoMWMxMyAwIDI2LTcgMjctOHYtMSIgZmlsbD0iI2ZhYWIwNyIvPjxwYXRoIGQ9Ik01NCAzMGMxIDItMSA1LTMgNS0xIDAtMy0yLTMtNC0xLTMgMS01IDMtNXMzIDEgMyA0TTY2IDMxYzEtMSAzLTUgOS0zbDIgMXYxbC0yIDFzLTQtMy03IDFoLTFsLTEtMSIvPjxwYXRoIGQ9Ik02MSA4M2gtMWwtMzMtMy0yIDIxYzIgMjIgMTUgMzYgMzUgMzdoMWMyMC0xIDMzLTE1IDM1LTM3bC0yLTIxLTMzIDMiIGZpbGw9IiNmZmYiLz48cGF0aCBkPSJNMzIgODF2MjJzMTAgMiAyMCAxVjg0bC0yMC0zIiBmaWxsPSIjZWIxOTIzIi8+PHBhdGggZD0iTTEwNiA2MHMtMjAgNy00NSA3aC0xYy0yNSAwLTQ0LTctNDUtN0w5IDc3YzE2IDQgMzYgOCA1MSA3aDFjMTUgMSAzNS0zIDUxLTd6bTAgMCIgZmlsbD0iI2ViMTkyMyIvPjwvc3ZnPg==)](http://shang.qq.com/wpa/qunwpa?idkey=aa5b2ba04d44820536a6aad31e3d59fb3e43fbed4cbdf1215b204ed61f22036e)

Copyright (c) 2018- The VIG Project.  
Copyright (c) 2018- The Monero Project.  
Copyright (c) 2012-2013 The Cryptonote developers.


## Status

| Compiler | Platform | Status | Coverage |
| --- | --- | --- | --- |
| gcc5 | Ubuntu 16.04 | [![CircleCI](https://circleci.com/gh/vigcoin/coin.svg?style=svg)](https://circleci.com/gh/vigcoin/coin) | [![codecov](https://codecov.io/gh/vigcoin/coin/branch/master/graph/badge.svg)](https://codecov.io/gh/vigcoin/coin)  |
| clang(llvm10) | Mac Os 10.14 | [![CircleCI](https://circleci.com/gh/vigcoin/coin.svg?style=svg)](https://circleci.com/gh/vigcoin/coin) | [![codecov](https://codecov.io/gh/vigcoin/coin/branch/master/graph/badge.svg)](https://codecov.io/gh/vigcoin/coin)  |

## Participate

1. Get yourself a wallet address:  
[![wallet](https://img.shields.io/badge/vigcoin-wallet-red.svg)](https://paper-wallet.vigcoin.org/)

2. Mining a coin from vig coin pools:  
[![mining](https://img.shields.io/badge/vigcoin-mining%20pool-white.svg?colorB=ff69b4)](http://pools.vigcoin.org/)

## Introduction

VIG Coin is a coin based on the cryptonote cryptocurrency protocol.

It's goal is to implement a coin with:

1. Fast exchanging speed
2. Anti-ASIC awareness
3. A light wallet api
4. Node with one Docker command
5. Offlinable Web based decentralized wallet
6. Decentralized exchanger
7. Anything that makes a coin better for everyone


## 参与

VIG COIN是一个开源，社区化的项目。非常欢迎各类开源贡献者的参与和支持。
同时VIG COIN也提供了回报机制鼓励更多人的参与。

详细可以参考：[wiki](https://github.com/vigcoin/wiki)

## Compiling

### Ubuntu

Currently only Ubuntu 16.04 is officially supported.

1. Package installation

```
sudo apt update && sudo apt install -y build-essential cmake libboost-all-dev libssl-dev libsodium-dev python-pip python-dev lcov
```
2. Building & testing

```
git clone https://github.com/vigcoin/coin.git
cd coin
mkdir -p build
cmake ..
make
make test
```

3. Auto building & testing
```
./build-debug.sh
```

### Mac Os

Mac Os Only support the latest version.

1. Package installation

```
brew install cmake boost libsodium openssl
```

2. export variables
``` 
export OPENSSL_ROOT_DIR=$(brew --prefix openssl)
```

3. Building & testing

```
git clone https://github.com/vigcoin/coin.git
cd coin
mkdir -p build
cmake ..
make
make test
```

## Donation

BTC:   
1MhPwnodHSTvptYLdF6d8c5NZkfqVdhfMS  
Monero: 4AeKZo1hZAg4MypBNesAsphdGJRwJtU2n68qbbeeNtH8QgdFRJVpQ3tNpXy6KtsKekXzRTMpGZjRWb2MKRxDc7w86rcZJZJ

## License

GPL v3 (VIG Code)

MIT(Origin/Monero Code)

[travis-image]: https://travis-ci.org/vigcoin/coin.svg?branch=master
[travis-url]: https://travis-ci.org/vigcoin/coin
[coveralls-image]: https://coveralls.io/repos/github/vigcoin/coin/badge.svg?branch=master
[coveralls-url]: https://coveralls.io/github/vigcoin/coin?branch=master
