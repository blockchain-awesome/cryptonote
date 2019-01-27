#!/usr/bin/env bash

if [ ! -d "src/cryptonote" ]; then
    mkdir -p src/cryptonote
fi

function move() {
mv src/$1 src/$2
}

dirs=(\
BlockchainExplorer blockchain_explorer \
HTTP http \
Miner miner \
Rpc rpc \
PaymentGate payment_gate \
PaymentGateService payment_gate_service \
SimpleWallet simplewallet \
Serialization serialization \
Daemon daemon \
Logging logging \
P2p p2p \
Platform platform \
System system \
Wallet wallet \
WalletLegacy wallet_legacy \
Common common \
Transfers transfers \
CryptoNoteCore cryptonote/core \
CryptoNoteProtocol cryptonote/protocol \
)

len=${#dirs[@]}
echo $len
for((i=0; i<$len; i+=2)); do
move ${dirs[$i]} ${dirs[$i+1]}
done

function replaceArray() {
cmd=$1
shift
replacements=("$@")
len=${#replacements[@]}
echo $len
for (( i=0; i<$len; i+=2 )); do
$cmd ${replacements[$i]} ${replacements[$i+1]}
done
}
# replaceArray ./replace-types.sh "${Namespaces[@]}"
replaceArray ./replace-types.sh "${dirs[@]}"

