// Copyright (c) 2018 The VIG Project
// Distributed under the GPL v3 software license

#pragma once

#include <boost/utility/value_init.hpp>
#include <cryptonote.h>

namespace cryptonote
{
const crypto::Hash NULL_HASH = boost::value_initialized<crypto::Hash>();
const crypto::PublicKey NULL_PUBLIC_KEY = boost::value_initialized<crypto::PublicKey>();
const crypto::SecretKey NULL_SECRET_KEY = boost::value_initialized<crypto::SecretKey>();

class Key
{
  public:
    template <typename T>
    static T zero()
    {
        return boost::value_initialized<T>();
    }
    static KeyPair generate();
};

} // namespace cryptonote
