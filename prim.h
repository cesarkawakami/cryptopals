#pragma once
#include "ByteStr.h"
#include "aes.h"
#include "config_int.h"
#include "filters.h"
#include "modes.h"
#include <config.h>
#include <rijndael.h>

namespace pals::prim {

inline static const int AES_BLOCK_SIZE = 16;

using _CryPaddingScheme = CryptoPP::BlockPaddingSchemeDef::BlockPaddingScheme;

bytestr::ByteStr transform(const bytestr::ByteStr &input,
                           auto tf,
                           _CryPaddingScheme padding_scheme = _CryPaddingScheme::DEFAULT_PADDING) {
    bytestr::ByteStr rv{};
    auto sink = new CryptoPP::StringSinkTemplate<std::vector<uint8_t>>(rv.data);
    auto filter = new CryptoPP::StreamTransformationFilter(tf, sink, padding_scheme);
    CryptoPP::StringSource ss(input.as_bytearr(), input.size(), true, filter);
    return rv;
}

bytestr::ByteStr aes_ecb_enc(const bytestr::ByteStr &clear_text, const bytestr::ByteStr &key);
bytestr::ByteStr aes_ecb_dec(const bytestr::ByteStr &cipher_text, const bytestr::ByteStr &key);

bytestr::ByteStr &pkcs7_pad_i(bytestr::ByteStr &bs, int block_size);
bytestr::ByteStr pkcs7_pad(const bytestr::ByteStr &bs, int block_size);
bytestr::ByteStr &pkcs7_unpad_i(bytestr::ByteStr &bs);
bytestr::ByteStr pkcs7_unpad(const bytestr::ByteStr &bs);

bytestr::ByteStr aes_cbc_enc(const bytestr::ByteStr &clear_text,
                             const bytestr::ByteStr &key,
                             const bytestr::ByteStr &iv);
bytestr::ByteStr aes_cbc_dec(const bytestr::ByteStr &cipher_text,
                             const bytestr::ByteStr &key,
                             const bytestr::ByteStr &iv);

} // namespace pals::prim
