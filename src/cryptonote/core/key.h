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
const PublicKey NULL_PUBLIC_KEY = Key::zero<PublicKey>();
const SecretKey NULL_SECRET_KEY = Key::zero<SecretKey>();

} // namespace cryptonote
