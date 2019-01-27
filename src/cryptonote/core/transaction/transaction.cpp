#include "transaction.h"
namespace cryptonote
{
uint64_t Transaction::getInputAmount()
{
  uint64_t amount = 0;
  for (auto item : m_transaction.inputs)
  {
    amount += getInputAmount(item);
  }
  return amount;
}

uint64_t Transaction::getOutputAmount()
{
  uint64_t amount = 0;
  for (auto item : m_transaction.outputs)
  {
    amount += item.amount;
  }
  return amount;
}

uint64_t Transaction::getInputAmount(const transaction_input_t &in)
{
  if (in.type() == typeid(key_input_t))
  {
    return boost::get<key_input_t>(in).amount;
  }
  if (in.type() == typeid(multi_signature_input_t))
  {
    return boost::get<multi_signature_input_t>(in).amount;
  }
  return 0;
}

uint64_t Transaction::getMixin(uint64_t mixin)
{
  for (auto in : m_transaction.inputs)
  {
    if (in.type() != typeid(key_input_t))
    {
      continue;
    }
    key_input_t ki = boost::get<key_input_t>(in);

    uint64_t size = ki.outputIndexes.size();
    if (mixin < size)
    {
      mixin = size;
    }
  }
  return mixin;
}

void Transaction::getInputDetails(std::vector<transaction_input_details_t> inputs)
{

  for (auto in : m_transaction.inputs)
  {
    transaction_input_details_t tid;
    if (in.type() == typeid(key_input_t))
    {
      transaction_input_to_key_details_t details;
      key_input_t kinput = boost::get<key_input_t>(in);

      std::vector<std::pair<crypto::hash_t, size_t>> outputReferences;
      outputReferences.reserve(kinput.outputIndexes.size());
      details.outputIndexes = kinput.outputIndexes;

      std::vector<uint32_t> res = kinput.outputIndexes;
      for (size_t i = 1; i < res.size(); i++)
      {
        res[i] += res[i - 1];
      }
      std::vector<uint32_t> globalIndexes = res;

      details.mixin = kinput.outputIndexes.size();
      // details.output.number = outputReferences.back().second;
      // details.output.transactionHash = outputReferences.back().first;
      tid.input = details;
    }
    inputs.push_back(tid);
  }
}
} // namespace cryptonote