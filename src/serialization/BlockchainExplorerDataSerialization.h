// Copyright (c) 2012-2017, The cryptonote developers, The Bytecoin developers
//
// This file is part of Bytecoin.
//
// Bytecoin is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Bytecoin is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Bytecoin.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include "BlockchainExplorerData.h"

#include "serialization/ISerializer.h"

namespace cryptonote {

void serialize(transaction_output_details_t& output, ISerializer& serializer);
void serialize(transaction_output_reference_details_t& outputReference, ISerializer& serializer);

// void serialize(BaseInputDetails& inputBase, ISerializer& serializer);
// void serialize(KeyInputDetails& inputToKey, ISerializer& serializer);
// void serialize(MultisignatureInputDetails& inputMultisig, ISerializer& serializer);
void serialize(transaction_input_details_t& input, ISerializer& serializer);

void serialize(transaction_extra_details_t& extra, ISerializer& serializer);
void serialize(transaction_details_t& transaction, ISerializer& serializer);

void serialize(block_details_t& block, ISerializer& serializer);

} //namespace cryptonote
