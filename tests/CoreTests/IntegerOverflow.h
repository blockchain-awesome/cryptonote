// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once 
#include "Chaingen.h"

struct gen_uint_overflow_base : public test_chain_unit_base
{
  gen_uint_overflow_base();

  bool check_TxVerificationContext(const cryptonote::TxVerificationContext& tvc, bool tx_added, size_t event_idx, const cryptonote::Transaction& tx);
  bool check_BlockVerificationContext(const cryptonote::BlockVerificationContext& bvc, size_t event_idx, const cryptonote::block_t& block);

  bool mark_last_valid_block(cryptonote::core& c, size_t ev_index, const std::vector<test_event_entry>& events);

private:
  size_t m_last_valid_block_event_idx;
};

struct gen_uint_overflow_1 : public gen_uint_overflow_base
{
  bool generate(std::vector<test_event_entry>& events) const;
};

struct gen_uint_overflow_2 : public gen_uint_overflow_base
{
  bool generate(std::vector<test_event_entry>& events) const;
};
