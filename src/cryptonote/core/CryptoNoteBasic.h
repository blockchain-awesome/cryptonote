// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <boost/utility/value_init.hpp>
#include <cryptonote.h>

namespace cryptonote {
  const crypto::Hash NULL_HASH = boost::value_initialized<crypto::Hash>();
  const crypto::PublicKey NULL_PUBLIC_KEY = boost::value_initialized<crypto::PublicKey>();
  const crypto::SecretKey NULL_SECRET_KEY = boost::value_initialized<crypto::SecretKey>();

  KeyPair generateKeyPair();
}
