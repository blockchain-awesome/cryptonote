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
} // namespace cryptonote