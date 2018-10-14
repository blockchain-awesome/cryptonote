#include "generated_transaction.h"
#include "cryptonote_core/CryptoNoteTools.h"

namespace cryptonote
{

class ISerializer;

GeneratedTransactionsIndex::GeneratedTransactionsIndex() : lastGeneratedTxNumber(0)
{
}

bool GeneratedTransactionsIndex::add(const Block &block)
{
  uint32_t blockHeight = boost::get<BaseInput>(block.baseTransaction.inputs.front()).blockIndex;

  if (index.size() != blockHeight)
  {
    return false;
  }

  bool status = index.emplace(blockHeight, lastGeneratedTxNumber + block.transactionHashes.size() + 1).second; //Plus miner tx
  if (status)
  {
    lastGeneratedTxNumber += block.transactionHashes.size() + 1;
  }
  return status;
}

bool GeneratedTransactionsIndex::remove(const Block &block)
{
  uint32_t blockHeight = boost::get<BaseInput>(block.baseTransaction.inputs.front()).blockIndex;

  if (blockHeight != index.size() - 1)
  {
    return false;
  }

  auto iter = index.find(blockHeight);
  assert(iter != index.end());
  index.erase(iter);

  if (blockHeight != 0)
  {
    iter = index.find(blockHeight - 1);
    assert(iter != index.end());
    lastGeneratedTxNumber = iter->second;
  }
  else
  {
    lastGeneratedTxNumber = 0;
  }

  return true;
}

bool GeneratedTransactionsIndex::find(uint32_t height, uint64_t &generatedTransactions)
{
  if (height > std::numeric_limits<uint32_t>::max())
  {
    return false;
  }
  auto iter = index.find(height);
  if (iter == index.end())
  {
    return false;
  }
  generatedTransactions = iter->second;
  return true;
}

void GeneratedTransactionsIndex::clear()
{
  index.clear();
}

void GeneratedTransactionsIndex::serialize(ISerializer &s)
{
  s(index, "index");
  s(lastGeneratedTxNumber, "lastGeneratedTxNumber");
}

} // namespace cryptonote