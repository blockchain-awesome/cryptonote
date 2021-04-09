#pragma once

#include "crypto/types.h"
#include "reader.h"
#include "writer.h"

#include "cryptonote.h"
#include "crypto.h"
#include "cryptonote/types.h"

using namespace cryptonote;

Reader &operator>>(Reader &i, block_header_t &v);
Reader &operator>>(Reader &i, block_t &v);
Reader &operator>>(Reader &i, key_pair_t &v);
Reader &operator>>(Reader &i, account_public_address_t &v);
Reader &operator>>(Reader &i, account_keys_t &v);
Reader &operator>>(Reader &i, core_state_info_t &v);
Reader &operator>>(Reader &i, block_entry_t &v);


Writer &operator<<(Writer &o, const block_header_t &v);
Writer &operator<<(Writer &o, const block_t &v);
Writer &operator<<(Writer &o, const key_pair_t &v);
Writer &operator<<(Writer &o, const account_public_address_t &v);
Writer &operator<<(Writer &o, const account_keys_t &v);
Writer &operator<<(Writer &o, const core_state_info_t &v);
Writer &operator<<(Writer &o, const block_entry_t &v);
