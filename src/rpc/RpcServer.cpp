// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "RpcServer.h"

#include <future>
#include <unordered_map>

// CryptoNote
#include "common/StringTools.h"
#include "cryptonote/core/CryptoNoteTools.h"
#include "cryptonote/core/core.h"
#include "cryptonote/core/account.h"
#include "cryptonote/core/IBlock.h"
#include "cryptonote/core/Miner.h"
#include "cryptonote/core/TransactionExtra.h"
// #include "cryptonote/core/blockchain.h"

#include "cryptonote/protocol/i_query.h"

#include "p2p/NetNode.h"

#include "CoreRpcServerErrorCodes.h"
#include "JsonRpc.h"
#include "version.h"

#undef ERROR

using namespace Logging;
using namespace crypto;
using namespace Common;

namespace cryptonote {

static inline void serialize(COMMAND_RPC_GET_BLOCKS_FAST::response& response, ISerializer &s) {
  KV_MEMBER(response.blocks)
  KV_MEMBER(response.start_height)
  KV_MEMBER(response.current_height)
  KV_MEMBER(response.status)
}

namespace {

template <typename Command>
RpcServer::HandlerFunction binMethod(bool (RpcServer::*handler)(typename Command::request const&, typename Command::response&)) {
  return [handler](RpcServer* obj, const HttpRequest& request, HttpResponse& response) {

    boost::value_initialized<typename Command::request> req;
    boost::value_initialized<typename Command::response> res;

    if (!loadFromBinaryKeyValue(static_cast<typename Command::request&>(req), request.getBody())) {
      return false;
    }

    bool result = (obj->*handler)(req, res);
    response.setBody(storeToBinaryKeyValue(res.data()));
    return result;
  };
}

template <typename Command>
RpcServer::HandlerFunction jsonMethod(bool (RpcServer::*handler)(typename Command::request const&, typename Command::response&)) {
  return [handler](RpcServer* obj, const HttpRequest& request, HttpResponse& response) {

    boost::value_initialized<typename Command::request> req;
    boost::value_initialized<typename Command::response> res;

    if (!loadFromJson(static_cast<typename Command::request&>(req), request.getBody())) {
      return false;
    }

    bool result = (obj->*handler)(req, res);
    response.setBody(storeToJson(res.data()));
    return result;
  };
}

}
  
std::unordered_map<std::string, RpcServer::RpcHandler<RpcServer::HandlerFunction>> RpcServer::s_handlers = {
  
  // binary handlers
  { "/getblocks.bin", { binMethod<COMMAND_RPC_GET_BLOCKS_FAST>(&RpcServer::on_get_blocks), false } },
  { "/queryblocks.bin", { binMethod<COMMAND_RPC_QUERY_BLOCKS>(&RpcServer::on_query_blocks), false } },
  { "/queryblockslite.bin", { binMethod<COMMAND_RPC_QUERY_BLOCKS_LITE>(&RpcServer::on_query_blocks_lite), false } },
  { "/get_o_indexes.bin", { binMethod<COMMAND_RPC_GET_TX_GLOBAL_OUTPUTS_INDEXES>(&RpcServer::on_get_indexes), false } },
  { "/getrandom_outs.bin", { binMethod<COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS>(&RpcServer::on_get_random_outs), false } },
  { "/get_pool_changes.bin", { binMethod<COMMAND_RPC_GET_POOL_CHANGES>(&RpcServer::onGetPoolChanges), false } },
  { "/get_pool_changes_lite.bin", { binMethod<COMMAND_RPC_GET_POOL_CHANGES_LITE>(&RpcServer::onGetPoolChangesLite), false } },

  // json handlers
  { "/getinfo", { jsonMethod<COMMAND_RPC_GET_INFO>(&RpcServer::on_get_info), true } },
  { "/getheight", { jsonMethod<COMMAND_RPC_GET_HEIGHT>(&RpcServer::on_get_height), true } },
  { "/gettransactions", { jsonMethod<COMMAND_RPC_GET_TRANSACTIONS>(&RpcServer::on_get_transactions), false } },
  { "/sendrawtransaction", { jsonMethod<COMMAND_RPC_SEND_RAW_TX>(&RpcServer::on_send_raw_tx), false } },
  { "/start_mining", { jsonMethod<COMMAND_RPC_START_MINING>(&RpcServer::on_start_mining), false } },
  { "/stop_mining", { jsonMethod<COMMAND_RPC_STOP_MINING>(&RpcServer::on_stop_mining), false } },
  { "/stop_daemon", { jsonMethod<COMMAND_RPC_STOP_DAEMON>(&RpcServer::on_stop_daemon), true } },

  // json rpc
  { "/json_rpc", { std::bind(&RpcServer::processJsonRpcRequest, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), true } }
};

RpcServer::RpcServer(System::Dispatcher& dispatcher, Logging::ILogger& log, core& c, NodeServer& p2p, const ICryptoNoteProtocolQuery& protocolQuery) :
  HttpServer(dispatcher, log), logger(log, "RpcServer"), m_core(c), m_p2p(p2p), m_protocolQuery(protocolQuery) {
}

void RpcServer::processRequest(const HttpRequest& request, HttpResponse& response) {
  auto url = request.getUrl();
  for (const auto& cors_domain: m_cors_domains) {
    response.addHeader("Access-Control-Allow-Origin", cors_domain);
  }

  auto it = s_handlers.find(url);
  if (it == s_handlers.end()) {
    response.setStatus(HttpResponse::STATUS_404);
    return;
  }

  if (!it->second.allowBusyCore && !isCoreReady()) {
    response.setStatus(HttpResponse::STATUS_500);
    response.setBody("Core is busy");
    return;
  }

  it->second.handler(this, request, response);
}

bool RpcServer::processJsonRpcRequest(const HttpRequest& request, HttpResponse& response) {

  using namespace JsonRpc;

  for (const auto& cors_domain: m_cors_domains) {
    response.addHeader("Access-Control-Allow-Origin", cors_domain);
  }
  response.addHeader("Content-Type", "application/json");

  JsonRpcRequest jsonRequest;
  JsonRpcResponse jsonResponse;

  try {
    logger(TRACE) << "JSON-RPC request: " << request.getBody();
    jsonRequest.parseRequest(request.getBody());
    jsonResponse.setId(jsonRequest.getId()); // copy id

    static std::unordered_map<std::string, RpcServer::RpcHandler<JsonMemberMethod>> jsonRpcHandlers = {
      { "getblockcount", { makeMemberMethod(&RpcServer::on_getblockcount), true } },
      { "on_getblockhash", { makeMemberMethod(&RpcServer::on_getblockhash), false } },
      { "getblocktemplate", { makeMemberMethod(&RpcServer::on_getblocktemplate), false } },
      { "getcurrencyid", { makeMemberMethod(&RpcServer::on_get_currency_id), true } },
      { "submitblock", { makeMemberMethod(&RpcServer::on_submitblock), false } },
      { "getlastblockheader", { makeMemberMethod(&RpcServer::on_get_last_block_header), false } },
      { "getblockheaderbyhash", { makeMemberMethod(&RpcServer::on_get_block_header_by_hash), false } },
      { "getblockheaderbyheight", { makeMemberMethod(&RpcServer::on_get_block_header_by_height), false } }
    };

    auto it = jsonRpcHandlers.find(jsonRequest.getMethod());
    if (it == jsonRpcHandlers.end()) {
      throw JsonRpcError(JsonRpc::errMethodNotFound);
    }

    if (!it->second.allowBusyCore && !isCoreReady()) {
      throw JsonRpcError(CORE_RPC_ERROR_CODE_CORE_BUSY, "Core is busy");
    }

    it->second.handler(this, jsonRequest, jsonResponse);

  } catch (const JsonRpcError& err) {
    jsonResponse.setError(err);
  } catch (const std::exception& e) {
    jsonResponse.setError(JsonRpcError(JsonRpc::errInternalError, e.what()));
  }

  response.setBody(jsonResponse.getBody());
  logger(TRACE) << "JSON-RPC response: " << jsonResponse.getBody();
  return true;
}

bool RpcServer::enableCors(const std::vector<std::string> domains) {
  m_cors_domains = domains;
  return true;
}

bool RpcServer::isCoreReady() {
  return m_p2p.get_payload_object().isSynchronized();
  // return m_core.currency().isTestnet() || m_p2p.get_payload_object().isSynchronized();
}

//
// Binary handlers
//

bool RpcServer::on_get_blocks(const COMMAND_RPC_GET_BLOCKS_FAST::request& req, COMMAND_RPC_GET_BLOCKS_FAST::response& res) {
  // TODO code duplication see InProcessNode::doGetNewBlocks()
  if (req.block_ids.empty()) {
    res.status = "Failed";
    return false;
  }

  if (req.block_ids.back() != m_core.getBlockIdByHeight(0)) {
    res.status = "Failed";
    return false;
  }

  uint32_t totalBlockCount;
  uint32_t startBlockIndex;
  std::vector<crypto::hash_t> supplement = m_core.findBlockchainSupplement(req.block_ids, COMMAND_RPC_GET_BLOCKS_FAST_MAX_COUNT, totalBlockCount, startBlockIndex);

  res.current_height = totalBlockCount;
  res.start_height = startBlockIndex;

  for (const auto& blockId : supplement) {
    assert(m_core.have_block(blockId));
    auto completeBlock = m_core.getBlock(blockId);
    assert(completeBlock != nullptr);

    res.blocks.resize(res.blocks.size() + 1);
    res.blocks.back().block = BinaryArray::toString(BinaryArray::to(completeBlock->getBlock()));

    res.blocks.back().txs.reserve(completeBlock->getTransactionCount());
    for (size_t i = 0; i < completeBlock->getTransactionCount(); ++i) {
      res.blocks.back().txs.push_back(BinaryArray::toString(BinaryArray::to(completeBlock->getTransaction(i))));
    }
  }

  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_query_blocks(const COMMAND_RPC_QUERY_BLOCKS::request& req, COMMAND_RPC_QUERY_BLOCKS::response& res) {
  uint32_t startHeight;
  uint32_t currentHeight;
  uint32_t fullOffset;

  if (!m_core.queryBlocks(req.block_ids, req.timestamp, startHeight, currentHeight, fullOffset, res.items)) {
    res.status = "Failed to perform query";
    return false;
  }

  res.start_height = startHeight;
  res.current_height = currentHeight;
  res.full_offset = fullOffset;
  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_query_blocks_lite(const COMMAND_RPC_QUERY_BLOCKS_LITE::request& req, COMMAND_RPC_QUERY_BLOCKS_LITE::response& res) {
  uint32_t startHeight;
  uint32_t currentHeight;
  uint32_t fullOffset;
  if (!m_core.queryBlocksLite(req.blockIds, req.timestamp, startHeight, currentHeight, fullOffset, res.items)) {
    res.status = "Failed to perform query";
    return false;
  }

  res.startHeight = startHeight;
  res.currentHeight = currentHeight;
  res.fullOffset = fullOffset;
  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_get_indexes(const COMMAND_RPC_GET_TX_GLOBAL_OUTPUTS_INDEXES::request& req, COMMAND_RPC_GET_TX_GLOBAL_OUTPUTS_INDEXES::response& res) {
  std::vector<uint32_t> outputIndexes;
  if (!m_core.get_tx_outputs_gindexs(req.txid, outputIndexes)) {
    res.status = "Failed";
    return true;
  }

  res.o_indexes.assign(outputIndexes.begin(), outputIndexes.end());
  res.status = CORE_RPC_STATUS_OK;
  logger(TRACE) << "COMMAND_RPC_GET_TX_GLOBAL_OUTPUTS_INDEXES: [" << res.o_indexes.size() << "]";
  return true;
}

bool RpcServer::on_get_random_outs(const COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS::request& req, COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS::response& res) {
  res.status = "Failed";
  if (!m_core.get_random_outs_for_amounts(req, res)) {
    return true;
  }

  res.status = CORE_RPC_STATUS_OK;

  std::stringstream ss;
  typedef COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS::outs_for_amount outs_for_amount;
  typedef COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS::out_entry out_entry;

  std::for_each(res.outs.begin(), res.outs.end(), [&](outs_for_amount& ofa)  {
    ss << "[" << ofa.amount << "]:";

    assert(ofa.outs.size() && "internal error: ofa.outs.size() is empty");

    std::for_each(ofa.outs.begin(), ofa.outs.end(), [&](out_entry& oe)
    {
      ss << oe.global_amount_index << " ";
    });
    ss << ENDL;
  });
  std::string s = ss.str();
  logger(TRACE) << "COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS: " << ENDL << s;
  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::onGetPoolChanges(const COMMAND_RPC_GET_POOL_CHANGES::request& req, COMMAND_RPC_GET_POOL_CHANGES::response& rsp) {
  rsp.status = CORE_RPC_STATUS_OK;
  std::vector<cryptonote::transaction_t> addedTransactions;
  rsp.isTailBlockActual = m_core.getPoolChanges(req.tailBlockId, req.knownTxsIds, addedTransactions, rsp.deletedTxsIds);
  for (auto& tx : addedTransactions) {
    binary_array_t txBlob;
    if (!BinaryArray::to(tx, txBlob)) {
      rsp.status = "Internal error";
      break;;
    }

    rsp.addedTxs.emplace_back(std::move(txBlob));
  }
  return true;
}


bool RpcServer::onGetPoolChangesLite(const COMMAND_RPC_GET_POOL_CHANGES_LITE::request& req, COMMAND_RPC_GET_POOL_CHANGES_LITE::response& rsp) {
  rsp.status = CORE_RPC_STATUS_OK;
  rsp.isTailBlockActual = m_core.getPoolChangesLite(req.tailBlockId, req.knownTxsIds, rsp.addedTxs, rsp.deletedTxsIds);

  return true;
}

//
// JSON handlers
//

bool RpcServer::on_get_info(const COMMAND_RPC_GET_INFO::request& req, COMMAND_RPC_GET_INFO::response& res) {
  res.already_generated_coins = m_core.getAlreadyGeneratedCoins();
  Blockchain& bc = m_core.getBlockChain();
  block_entry_t bet = bc.getLastBlock();
  res.block_major_version = config::get().block.version.major;
  res.block_minor_version = config::get().block.version.minor;
  res.last_block_difficulty = bc.blockDifficulty(bet.height);
  res.last_block_timestamp = bet.bl.timestamp;
  // res.last_block_reward = bet.bl.timestamp;
  res.min_tx_fee = cryptonote::parameters::MINIMUM_FEE;
  res.version = PROJECT_VERSION_LONG;

  res.height = m_core.get_current_blockchain_height();
  res.difficulty = m_core.getNextBlockDifficulty();
  res.tx_count = m_core.get_blockchain_total_transactions() - res.height; //without coinbase
  res.tx_pool_size = m_core.get_pool_transactions_count();
  res.alt_blocks_count = m_core.get_alternative_blocks_count();
  uint64_t total_conn = m_p2p.get_connections_count();
  res.outgoing_connections_count = m_p2p.get_outgoing_connections_count();
  res.incoming_connections_count = total_conn - res.outgoing_connections_count;
  res.white_peerlist_size = m_p2p.getPeerlistManager().get_white_peers_count();
  res.grey_peerlist_size = m_p2p.getPeerlistManager().get_gray_peers_count();
  res.last_known_block_index = std::max(static_cast<uint32_t>(1), m_protocolQuery.getObservedHeight()) - 1;
  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_get_height(const COMMAND_RPC_GET_HEIGHT::request& req, COMMAND_RPC_GET_HEIGHT::response& res) {
  res.height = m_core.get_current_blockchain_height();
  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_get_transactions(const COMMAND_RPC_GET_TRANSACTIONS::request& req, COMMAND_RPC_GET_TRANSACTIONS::response& res) {
  std::vector<hash_t> vh;
  for (const auto& tx_hex_str : req.txs_hashes) {
    binary_array_t b;
    if (!hex::fromString(tx_hex_str, b))
    {
      res.status = "Failed to parse hex representation of transaction hash";
      return true;
    }
    if (b.size() != sizeof(hash_t))
    {
      res.status = "Failed, size of data mismatch";
    }
    vh.push_back(*reinterpret_cast<const hash_t*>(b.data()));
  }
  std::list<hash_t> missed_txs;
  std::list<transaction_t> txs;
  m_core.getTransactions(vh, txs, missed_txs);

  for (auto& tx : txs) {
    res.txs_as_hex.push_back(hex::toString(BinaryArray::to(tx)));
  }

  for (const auto& miss_tx : missed_txs) {
    res.missed_tx.push_back(hex::podToString(miss_tx));
  }

  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_send_raw_tx(const COMMAND_RPC_SEND_RAW_TX::request& req, COMMAND_RPC_SEND_RAW_TX::response& res) {
  binary_array_t tx_blob;
  if (!hex::fromString(req.tx_as_hex, tx_blob))
  {
    logger(INFO) << "[on_send_raw_tx]: Failed to parse tx from hexbuff: " << req.tx_as_hex;
    res.status = "Failed";
    return true;
  }

  tx_verification_context_t tvc = boost::value_initialized<tx_verification_context_t>();
  if (!m_core.handle_incoming_tx(tx_blob, tvc, false))
  {
    logger(INFO) << "[on_send_raw_tx]: Failed to process tx";
    res.status = "Failed";
    return true;
  }

  if (tvc.m_verifivation_failed)
  {
    logger(INFO) << "[on_send_raw_tx]: tx verification failed";
    res.status = "Failed";
    return true;
  }

  if (!tvc.m_should_be_relayed)
  {
    logger(INFO) << "[on_send_raw_tx]: tx accepted, but not relayed";
    res.status = "Not relayed";
    return true;
  }


  NOTIFY_NEW_TRANSACTIONS::request r;
  r.txs.push_back(BinaryArray::toString(tx_blob));
  m_core.get_protocol()->relay_transactions(r);
  //TODO: make sure that tx has reached other nodes here, probably wait to receive reflections from other nodes
  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_start_mining(const COMMAND_RPC_START_MINING::request& req, COMMAND_RPC_START_MINING::response& res) {
  account_public_address_t adr;
  if (!Account::parseAddress(req.miner_address, adr)) {
    res.status = "Failed, wrong address";
    return true;
  }

  if (!m_core.get_miner().start(adr, static_cast<size_t>(req.threads_count))) {
    res.status = "Failed, mining not started";
    return true;
  }

  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_stop_mining(const COMMAND_RPC_STOP_MINING::request& req, COMMAND_RPC_STOP_MINING::response& res) {
  if (!m_core.get_miner().stop()) {
    res.status = "Failed, mining not stopped";
    return true;
  }
  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_stop_daemon(const COMMAND_RPC_STOP_DAEMON::request& req, COMMAND_RPC_STOP_DAEMON::response& res) {
  // if (m_core.currency().isTestnet()) {
  //   m_p2p.sendStopSignal();
  //   res.status = CORE_RPC_STATUS_OK;
  // } else {
    res.status = CORE_RPC_ERROR_CODE_INTERNAL_ERROR;
    return false;
  // }
  // return true;
}

//------------------------------------------------------------------------------------------------------------------------------
// JSON RPC methods
//------------------------------------------------------------------------------------------------------------------------------
bool RpcServer::on_getblockcount(const COMMAND_RPC_GETBLOCKCOUNT::request& req, COMMAND_RPC_GETBLOCKCOUNT::response& res) {
  res.count = m_core.get_current_blockchain_height();
  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_getblockhash(const COMMAND_RPC_GETBLOCKHASH::request& req, COMMAND_RPC_GETBLOCKHASH::response& res) {
  if (req.size() != 1) {
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_WRONG_PARAM, "Wrong parameters, expected height" };
  }

  uint32_t h = static_cast<uint32_t>(req[0]);
  crypto::hash_t blockId = m_core.getBlockIdByHeight(h);
  if (blockId == NULL_HASH) {
    throw JsonRpc::JsonRpcError{ 
      CORE_RPC_ERROR_CODE_TOO_BIG_HEIGHT,
      std::string("To big height: ") + std::to_string(h) + ", current blockchain height = " + std::to_string(m_core.get_current_blockchain_height())
    };
  }

  res = hex::podToString(blockId);
  return true;
}

namespace {
  uint64_t slow_memmem(void* start_buff, size_t buflen, void* pat, size_t patlen)
  {
    void* buf = start_buff;
    void* end = (char*)buf + buflen - patlen;
    while ((buf = memchr(buf, ((char*)pat)[0], buflen)))
    {
      if (buf>end)
        return 0;
      if (memcmp(buf, pat, patlen) == 0)
        return (char*)buf - (char*)start_buff;
      buf = (char*)buf + 1;
    }
    return 0;
  }
}

bool RpcServer::on_getblocktemplate(const COMMAND_RPC_GETBLOCKTEMPLATE::request& req, COMMAND_RPC_GETBLOCKTEMPLATE::response& res) {
  if (req.reserve_size > TX_EXTRA_NONCE_MAX_COUNT) {
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_TOO_BIG_RESERVE_SIZE, "To big reserved size, maximum 255" };
  }

  account_public_address_t acc = boost::value_initialized<account_public_address_t>();

  if (!req.wallet_address.size() || !Account::parseAddress(req.wallet_address, acc)) {
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_WRONG_WALLET_ADDRESS, "Failed to parse wallet address" };
  }

  block_t b = boost::value_initialized<block_t>();
  binary_array_t blob_reserve;
  blob_reserve.resize(req.reserve_size, 0);
  if (!m_core.get_block_template(b, acc, res.difficulty, res.height, blob_reserve)) {
    logger(ERROR) << "Failed to create block template";
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Internal error: failed to create block template" };
  }

  binary_array_t block_blob = BinaryArray::to(b);
  public_key_t tx_pub_key = cryptonote::getTransactionPublicKeyFromExtra(b.baseTransaction.extra);
  if (tx_pub_key == NULL_PUBLIC_KEY) {
    logger(ERROR) << "Failed to find tx pub key in coinbase extra";
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Internal error: failed to find tx pub key in coinbase extra" };
  }

  if (0 < req.reserve_size) {
    res.reserved_offset = slow_memmem((void*)block_blob.data(), block_blob.size(), &tx_pub_key, sizeof(tx_pub_key));
    if (!res.reserved_offset) {
      logger(ERROR) << "Failed to find tx pub key in blockblob";
      throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Internal error: failed to create block template" };
    }
    res.reserved_offset += sizeof(tx_pub_key) + 3; //3 bytes: tag for TX_EXTRA_TAG_PUBKEY(1 byte), tag for TX_EXTRA_NONCE(1 byte), counter in TX_EXTRA_NONCE(1 byte)
    if (res.reserved_offset + req.reserve_size > block_blob.size()) {
      logger(ERROR) << "Failed to calculate offset for reserved bytes";
      throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Internal error: failed to create block template" };
    }
  } else {
    res.reserved_offset = 0;
  }

  res.blocktemplate_blob = hex::toString(block_blob);
  res.status = CORE_RPC_STATUS_OK;

  return true;
}

bool RpcServer::on_get_currency_id(const COMMAND_RPC_GET_CURRENCY_ID::request& /*req*/, COMMAND_RPC_GET_CURRENCY_ID::response& res) {
  hash_t currencyId = m_core.currency().genesisBlockHash();
  res.currency_id_blob = hex::podToString(currencyId);
  return true;
}

bool RpcServer::on_submitblock(const COMMAND_RPC_SUBMITBLOCK::request& req, COMMAND_RPC_SUBMITBLOCK::response& res) {
  if (req.size() != 1) {
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_WRONG_PARAM, "Wrong param" };
  }

  binary_array_t blockblob;
  if (!hex::fromString(req[0], blockblob)) {
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_WRONG_BLOCKBLOB, "Wrong block blob" };
  }

  block_verification_context_t bvc = boost::value_initialized<block_verification_context_t>();

  m_core.handle_incoming_block_blob(blockblob, bvc, true, true);

  if (!bvc.m_added_to_main_chain) {
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_BLOCK_NOT_ACCEPTED, "Block not accepted" };
  }

  res.status = CORE_RPC_STATUS_OK;
  return true;
}


namespace {
  uint64_t get_block_reward(const block_t& blk) {
    uint64_t reward = 0;
    for (const transaction_output_t& out : blk.baseTransaction.outputs) {
      reward += out.amount;
    }
    return reward;
  }
}

void RpcServer::fill_block_header_response(const block_t& blk, bool orphan_status, uint64_t height, const hash_t& hash, block_header_response& responce) {
  responce.major_version = blk.majorVersion;
  responce.minor_version = blk.minorVersion;
  responce.timestamp = blk.timestamp;
  responce.prev_hash = hex::podToString(blk.previousBlockHash);
  responce.nonce = blk.nonce;
  responce.orphan_status = orphan_status;
  responce.height = height;
  responce.depth = m_core.get_current_blockchain_height() - height - 1;
  responce.hash = hex::podToString(hash);
  m_core.getBlockDifficulty(static_cast<uint32_t>(height), responce.difficulty);
  responce.reward = get_block_reward(blk);
}

bool RpcServer::on_get_last_block_header(const COMMAND_RPC_GET_LAST_BLOCK_HEADER::request& req, COMMAND_RPC_GET_LAST_BLOCK_HEADER::response& res) {
  uint32_t last_block_height;
  hash_t last_block_hash;
  
  m_core.get_blockchain_top(last_block_height, last_block_hash);

  block_t last_block;
  if (!m_core.getBlockByHash(last_block_hash, last_block)) {
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Internal error: can't get last block hash." };
  }
  
  fill_block_header_response(last_block, false, last_block_height, last_block_hash, res.block_header);
  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_get_block_header_by_hash(const COMMAND_RPC_GET_BLOCK_HEADER_BY_HASH::request& req, COMMAND_RPC_GET_BLOCK_HEADER_BY_HASH::response& res) {
  hash_t block_hash;

  if (!parse_hash256(req.hash, block_hash)) {
    throw JsonRpc::JsonRpcError{
      CORE_RPC_ERROR_CODE_WRONG_PARAM,
      "Failed to parse hex representation of block hash. Hex = " + req.hash + '.' };
  }

  block_t blk;
  if (!m_core.getBlockByHash(block_hash, blk)) {
    throw JsonRpc::JsonRpcError{
      CORE_RPC_ERROR_CODE_INTERNAL_ERROR,
      "Internal error: can't get block by hash. hash_t = " + req.hash + '.' };
  }

  if (blk.baseTransaction.inputs.front().type() != typeid(base_input_t)) {
    throw JsonRpc::JsonRpcError{
      CORE_RPC_ERROR_CODE_INTERNAL_ERROR,
      "Internal error: coinbase transaction in the block has the wrong type" };
  }

  uint64_t block_height = boost::get<base_input_t>(blk.baseTransaction.inputs.front()).blockIndex;
  fill_block_header_response(blk, false, block_height, block_hash, res.block_header);
  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_get_block_header_by_height(const COMMAND_RPC_GET_BLOCK_HEADER_BY_HEIGHT::request& req, COMMAND_RPC_GET_BLOCK_HEADER_BY_HEIGHT::response& res) {
  if (m_core.get_current_blockchain_height() <= req.height) {
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_TOO_BIG_HEIGHT,
      std::string("To big height: ") + std::to_string(req.height) + ", current blockchain height = " + std::to_string(m_core.get_current_blockchain_height()) };
  }

  hash_t block_hash = m_core.getBlockIdByHeight(static_cast<uint32_t>(req.height));
  block_t blk;
  if (!m_core.getBlockByHash(block_hash, blk)) {
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR,
      "Internal error: can't get block by height. Height = " + std::to_string(req.height) + '.' };
  }

  fill_block_header_response(blk, false, req.height, block_hash, res.block_header);
  res.status = CORE_RPC_STATUS_OK;
  return true;
}


//------------------------------------------------------------------------------------------------------------------------------
// JSON RPC methods
//------------------------------------------------------------------------------------------------------------------------------

bool RpcServer::f_on_blocks_list_json(const F_COMMAND_RPC_GET_BLOCKS_LIST::request &req, F_COMMAND_RPC_GET_BLOCKS_LIST::response &res)
{
  Blockchain &bc = m_core.getBlockChain();

  if (bc.getHeight() < req.height)
  {
    throw JsonRpc::JsonRpcError{CORE_RPC_ERROR_CODE_TOO_BIG_HEIGHT,
                                std::string("To big height: ") + std::to_string(req.height) + ", current blockchain height = " + std::to_string(bc.getHeight())};
  }

  uint32_t print_blocks_count = 30;
  uint32_t last_height = req.height - print_blocks_count;
  if (req.height <= print_blocks_count)
  {
    last_height = 0;
  }

  for (uint32_t i = req.height; i >= last_height; i--)
  {
    hash_t block_hash = bc.getBlockIdByHeight(static_cast<uint32_t>(i));
    block_t blk;
    if (bc.getBlockByHash(block_hash, blk))
    {
      throw JsonRpc::JsonRpcError{
          CORE_RPC_ERROR_CODE_INTERNAL_ERROR,
          "Internal error: can't get block by height. Height = " + std::to_string(i) + '.'};
    }
    f_block_short_response block_short;
    block_short.cumul_size = 0;
    block_short.timestamp = blk.timestamp;
    block_short.height = i;
    block_short.hash = hex::podToString(block_hash);
    block_short.tx_count = blk.transactionHashes.size() + 1;

    res.blocks.push_back(block_short);

    if (i == 0)
    {
      break;
    }
  }

  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::f_on_block_json(const F_COMMAND_RPC_GET_BLOCK_DETAILS::request &req, F_COMMAND_RPC_GET_BLOCK_DETAILS::response &res)
{
  hash_t hash;
  Blockchain &bc = m_core.getBlockChain();
  uint32_t height = boost::lexical_cast<uint32_t>(req.hash);

  try
  {
    hash = bc.getBlockIdByHeight(height);
  }
  catch (boost::bad_lexical_cast &)
  {
    if (!parse_hash256(req.hash, hash))
    {
      throw JsonRpc::JsonRpcError{
          CORE_RPC_ERROR_CODE_WRONG_PARAM,
          "Failed to parse hex representation of block hash. Hex = " + req.hash + '.'};
    }
  }

  if (!bc.haveBlock(hash))
  {
    throw JsonRpc::JsonRpcError{
        CORE_RPC_ERROR_CODE_INTERNAL_ERROR,
        "Internal error: can't get block by hash. hash_t = " + req.hash + '.'};
  }
  block_entry_t be = bc.getBlock(height);
  block_t &blk = be.bl;
  // block_details_t blkDetails = bc.getBlockDetails(hash);
    block_header_response block_header;
    res.block.height = height;

    res.block.major_version = blk.majorVersion;
    res.block.minor_version = blk.minorVersion;
    res.block.timestamp = blk.timestamp;
    res.block.prev_hash = hex::podToString(bc.getBlockIdByHeight(height - 1));
    res.block.nonce = blk.nonce;
    res.block.hash = hex::podToString(hash);
    res.block.depth = bc.getHeight() - res.block.height;
    res.block.difficulty = be.cumulative_difficulty;
    // res.block.transactionsCumulativeSize = blkDetails.transactionsCumulativeSize;
    res.block.alreadyGeneratedCoins = be.already_generated_coins;
  //   res.block.alreadyGeneratedTransactions = blkDetails.alreadyGeneratedTransactions;
  //   res.block.reward = block_header.reward;
  //   res.block.sizeMedian = blkDetails.sizeMedian;
    res.block.blockSize = be.block_cumulative_size;
  //   res.block.orphan_status = blkDetails.isAlternative;

  //   uint64_t maxReward = 0;
  //   uint64_t currentReward = 0;
  //   int64_t emissionChange = 0;
  //   size_t blockGrantedFullRewardZone = m_core.getCurrency().blockGrantedFullRewardZoneByBlockVersion(block_header.major_version);
  //   res.block.effectiveSizeMedian = std::max(res.block.sizeMedian, blockGrantedFullRewardZone);

  //   res.block.baseReward = blkDetails.baseReward;
    // res.block.penalty = be.bl.;

    // Base transaction adding
    f_transaction_short_response transaction_short;
    transaction_short.hash = hex::podToString(BinaryArray::objectHash(blk.baseTransaction));
    transaction_short.fee = 0;
    transaction_short.amount_out = getOutputAmount(blk.baseTransaction);
    transaction_short.size = BinaryArray::size(blk.baseTransaction);
    res.block.transactions.push_back(transaction_short);

    std::list<crypto::hash_t> missed_txs;
    std::list<transaction_t> txs;
    bc.getTransactions(blk.transactionHashes, txs, missed_txs);

    res.block.totalFeeAmount = 0;

    for (const transaction_t& tx : txs) {
      f_transaction_short_response transaction_short;
      uint64_t amount_in = getInputAmount(tx);
      uint64_t amount_out = getOutputAmount(tx);

      transaction_short.hash = hex::podToString(BinaryArray::objectHash(tx));
      transaction_short.fee = amount_in - amount_out;
      transaction_short.amount_out = amount_out;
      transaction_short.size = BinaryArray::size(tx);
      res.block.transactions.push_back(transaction_short);

      res.block.totalFeeAmount += transaction_short.fee;
    }
  res.status = CORE_RPC_STATUS_OK;
  return true;
}

// bool RpcServer::f_on_transaction_json(const F_COMMAND_RPC_GET_TRANSACTION_DETAILS::request& req, F_COMMAND_RPC_GET_TRANSACTION_DETAILS::response& res) {
//   hash_t hash;

//   if (!parse_hash256(req.hash, hash)) {
//     throw JsonRpc::JsonRpcError{
//       CORE_RPC_ERROR_CODE_WRONG_PARAM,
//       "Failed to parse hex representation of transaction hash. Hex = " + req.hash + '.' };
//   }

//   std::vector<crypto::hash_t> tx_ids;
//   tx_ids.push_back(hash);

//   std::vector<crypto::hash_t> missed_txs;
//   std::vector<binary_array_t> txs;
//   m_core.getTransactions(tx_ids, txs, missed_txs);

//   if (1 == txs.size()) {
//     transaction_t transaction;
//     if (!fromBinaryArray(transaction, txs.front())) {
//       throw std::runtime_error("Couldn't deserialize transaction");
//     }
//     res.tx = transaction;
//   } else {
//     throw JsonRpc::JsonRpcError{
//       CORE_RPC_ERROR_CODE_WRONG_PARAM,
//       "transaction wasn't found. hash_t = " + req.hash + '.' };
//   }
//   TransactionDetails transactionDetails = m_core.getTransactionDetails(hash);

//   crypto::hash_t blockHash;
//   if (transactionDetails.inBlockchain) {
//     uint32_t blockHeight = transactionDetails.blockIndex;
//     if (!blockHeight) {
//       throw JsonRpc::JsonRpcError{
//         CORE_RPC_ERROR_CODE_INTERNAL_ERROR,
//         "Internal error: can't get transaction by hash. hash_t = " + Common::podToHex(hash) + '.' };
//     }
//     blockHash = m_core.getBlockHashByIndex(blockHeight);
//     block_t blk = m_core.getBlockByHash(blockHash);
//     block_details_t blkDetails = m_core.getBlockDetails(blockHash);

//     f_block_short_response block_short;

//     block_short.cumul_size = blkDetails.blockSize;
//     block_short.timestamp = blk.timestamp;
//     block_short.height = blockHeight;
//     block_short.hash = Common::podToHex(blockHash);
//     block_short.tx_count = blk.transactionHashes.size() + 1;
//     res.block = block_short;
//   }

//   uint64_t amount_in = getInputAmount(res.tx);
//   uint64_t amount_out = getOutputAmount(res.tx);

//   res.txDetails.hash = Common::podToHex(BinaryArray::objectHash(res.tx));
//   res.txDetails.fee = amount_in - amount_out;
//   if (amount_in == 0)
//     res.txDetails.fee = 0;
//   res.txDetails.amount_out = amount_out;
//   res.txDetails.size = getObjectBinarySize(res.tx);

//   uint64_t mixin;
//   if (!f_getMixin(res.tx, mixin)) {
//     return false;
//   }
//   res.txDetails.mixin = mixin;

//   crypto::hash_t paymentId;
//   if (cryptonote::getPaymentIdFromTxExtra(res.tx.extra, paymentId)) {
//     res.txDetails.paymentId = Common::podToHex(paymentId);
//   } else {
//     res.txDetails.paymentId = "";
//   }

//   res.status = CORE_RPC_STATUS_OK;
//   return true;
// }


bool RpcServer::f_on_transactions_pool_json(const F_COMMAND_RPC_GET_POOL::request& req, F_COMMAND_RPC_GET_POOL::response& res) {
  auto pool = m_core.getPoolTransactions();
  for (const transaction_t tx : pool) {
    f_transaction_short_response transaction_short;
    uint64_t amount_in = getInputAmount(tx);
    uint64_t amount_out = getOutputAmount(tx);

    transaction_short.hash = hex::podToString(BinaryArray::objectHash(tx));
    transaction_short.fee = amount_in - amount_out;
    transaction_short.amount_out = amount_out;
    transaction_short.size = BinaryArray::size(tx);
    res.transactions.push_back(transaction_short);
  }

  res.status = CORE_RPC_STATUS_OK;
  return true;
}

// bool RpcServer::f_getMixin(const transaction_t& transaction, uint64_t& mixin) {
//   mixin = 0;
//   for (const transaction_input_t& txin : transaction.inputs) {
//     if (txin.type() != typeid(KeyInput)) {
//       continue;
//     }
//     uint64_t currentMixin = boost::get<KeyInput>(txin).outputIndexes.size();
//     if (currentMixin > mixin) {
//       mixin = currentMixin;
//     }
//   }
//   return true;
// }




}
