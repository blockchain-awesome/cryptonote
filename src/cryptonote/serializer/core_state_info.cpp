
#include "core_state_info.h"

namespace cryptonote
{

  void csi_t::serialize(ISerializer &s)
  {
    KV_MEMBER(tx_pool_size);
    KV_MEMBER(blockchain_height);
    KV_MEMBER(mining_speed);
    KV_MEMBER(alternative_blocks);
    KV_MEMBER(top_block_id_str);
  };
}
