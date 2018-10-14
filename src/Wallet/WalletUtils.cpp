// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "WalletUtils.h"

#include "CryptoNote.h"

namespace cryptonote {

bool validateAddress(const std::string& address, const cryptonote::Currency& currency) {
  cryptonote::AccountPublicAddress ignore;
  return currency.parseAccountAddressString(address, ignore);
}

}
