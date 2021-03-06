#!/usr/bin/env bash

# #Namespace Update

Namespaces=(\
Crypto crypto \
CryptoNote cryptonote
)

# #Block Info update

# ./replace-types.sh BlockFullInfo block_full_info_t
# ./replace-types.sh BlockShortInfo block_short_info_t
# ./replace-types.sh BlockHeader block_header_t
# ./replace-types.sh BlockTemplate block_t
# ./replace-types.sh BlockDetails block_details_t

# ./replace-types.sh Transaction transaction_t
# ./replace-types.sh TransactionPrefix transaction_prefix_t
# ./replace-types.sh TransactionInput transaction_input_t
# ./replace-types.sh TransactionOutput transaction_output_t
# ./replace-types.sh TransactionPrefixInfo transaction_prefix_info_t
# ./replace-types.sh TransactionDetails transaction_details_t
# ./replace-types.sh TransactionRemoveReason transaction_remove_reason_t
# ./replace-types.sh TransactionOutputToKeyDetails transaction_output_to_key_details_t
# ./replace-types.sh TransactionOutputDetails transaction_output_details_t
# ./replace-types.sh TransactionOutputReferenceDetails transaction_output_reference_details_t
# ./replace-types.sh TransactionExtraDetails transaction_extra_details_t
# ./replace-types.sh TransactionOutputMultisignatureDetails transaction_output_multi_signature_details_t

# ./replace-types.sh BinaryArray binary_array_t
# ./replace-types.sh Signature signature_t
# ./replace-types.sh KeyDerivation key_derivation_t
# ./replace-types.sh SecretKey secret_key_t
# ./replace-types.sh PublicKey public_key_t
# ./replace-types.sh KeyImage key_image_t
# ./replace-types.sh Hash hash_t
# ./replace-types.sh Difficulty difficulty_t
# ./replace-types.sh EllipticCurveScalar elliptic_curve_scalar_t
# ./replace-types.sh EllipticCurvePoint elliptic_curve_point_t
# ./replace-types.sh EllipticCurveScalar elliptic_curve_scalar_t
# ./replace-types.sh EllipticCurveScalar elliptic_curve_scalar_t
# ./replace-types.sh EllipticCurveScalar elliptic_curve_scalar_t

replacements1=(\
# getObjectHash BinaryArray::objectHash \
# getObjectBinarySize BinaryArray::size \
TransactionInputGenerateDetails transaction_input_generate_details_t \
TransactionInputToKeyDetails transaction_input_to_key_details_t \
TransactionInputMultisignatureDetails transaction_input_multisignature_details_t
)
function replaceArray() {
cmd=$1
shift
replacements=("$@")
len=${#replacements[@]}
echo $len
for (( i=0; i<$len; i+=2 )); do
$cmd ${replacements[$i]} ${replacements[$i+1]}
done
}
# replaceArray ./replace-types.sh "${Namespaces[@]}"
replaceArray ./replace-types.sh "${replacements1[@]}"
