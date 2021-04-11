// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <stdexcept>
#include <algorithm>
#include <string>

#include "IWalletLegacy.h"
#include "stream/crypto.h"
#include "stream/cryptonote.h"
#include "stream/transaction.h"

namespace cryptonote
{
  class ISerializer;

  struct UnconfirmedTransferDetails;
  struct WalletLegacyTransaction;
  struct WalletLegacyTransfer;

  void serialize(UnconfirmedTransferDetails &utd, ISerializer &serializer);
  void serialize(WalletLegacyTransaction &txi, ISerializer &serializer);
  void serialize(WalletLegacyTransfer &tr, ISerializer &serializer);

  Reader &operator>>(Reader &i, UnconfirmedTransferDetails &v);
  Writer &operator<<(Writer &o, const UnconfirmedTransferDetails &v);
  Reader &operator>>(Reader &i, WalletLegacyTransaction &v);
  Writer &operator<<(Writer &o, const WalletLegacyTransaction &v);
  Reader &operator>>(Reader &i, WalletLegacyTransfer &v);
  Writer &operator<<(Writer &o, const WalletLegacyTransfer &v);
}
