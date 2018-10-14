// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "WalletFactory.h"

#include "NodeRpcProxy/NodeRpcProxy.h"
#include "Wallet/WalletGreen.h"
#include "cryptonote/core/Currency.h"

#include <stdlib.h>
#include <future>

namespace PaymentService {

WalletFactory WalletFactory::factory;

WalletFactory::WalletFactory() {
}

WalletFactory::~WalletFactory() {
}

cryptonote::IWallet* WalletFactory::createWallet(const cryptonote::Currency& currency, cryptonote::INode& node, System::Dispatcher& dispatcher) {
  cryptonote::IWallet* wallet = new cryptonote::WalletGreen(dispatcher, currency, node);
  return wallet;
}

}
