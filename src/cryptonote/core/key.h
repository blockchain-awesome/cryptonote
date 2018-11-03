// Copyright (c) 2018 The VIG Project
// Distributed under the GPL v3 software license

#pragma once

#include <boost/utility/value_init.hpp>
#include <cryptonote.h>

using namespace crypto;

namespace cryptonote
{
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

const Hash NULL_HASH = Key::zero<Hash>();
const public_key_t NULL_PUBLIC_KEY = Key::zero<public_key_t>();
const secret_key_t NULL_SECRET_KEY = Key::zero<secret_key_t>();

} // namespace cryptonote
