
#pragma once

#include "crypto/types.h"
#include "reader.h"
#include "writer.h"

#include "cryptonote.h"
#include "crypto.h"
#include "cryptonote/types.h"

using namespace cryptonote;

namespace stream
{
  namespace cryptonote
  {
    // Basic
    Reader &operator>>(Reader &i, base_input_t &v);
    Reader &operator>>(Reader &i, key_input_t &v);
    Reader &operator>>(Reader &i, multi_signature_input_t &v);
    Reader &operator>>(Reader &i, transaction_input_t &v);
    Reader &operator>>(Reader &i, key_output_t &v);
    Reader &operator>>(Reader &i, multi_signature_output_t &v);
    Reader &operator>>(Reader &i, transaction_output_target_t &v);
    Reader &operator>>(Reader &i, transaction_output_t &v);
    Reader &operator>>(Reader &i, transaction_prefix_t &v);
    Reader &operator>>(Reader &i, transaction_t &v);

    Writer &operator<<(Writer &o, const base_input_t &v);
    Writer &operator<<(Writer &o, const key_input_t &v);
    Writer &operator<<(Writer &o, const multi_signature_input_t &v);
    Writer &operator<<(Writer &o, const transaction_input_t &v);
    Writer &operator<<(Writer &o, const key_output_t &v);
    Writer &operator<<(Writer &o, const multi_signature_output_t &v);
    Writer &operator<<(Writer &o, const transaction_output_target_t &v);
    Writer &operator<<(Writer &o, const transaction_output_t &v);
    Writer &operator<<(Writer &o, const transaction_prefix_t &v);
    Writer &operator<<(Writer &o, const transaction_t &v);

    // Extra
    Reader &operator>>(Reader &i, transaction_index_t &v);
    Reader &operator>>(Reader &i, transaction_map_t &v);
    Reader &operator>>(Reader &i, multisignature_output_usage_t &v);
    Reader &operator>>(Reader &i, transaction_entry_t &v);

    Writer &operator<<(Writer &o, const transaction_index_t &v);
    Writer &operator<<(Writer &o, const transaction_map_t &v);
    Writer &operator<<(Writer &o, const multisignature_output_usage_t &v);
    Writer &operator<<(Writer &o, const transaction_entry_t &v);
  }
}