#pragma once
#include "ByteStr.h"
#include "aes.h"
#include "config_int.h"
#include "filters.h"
#include "modes.h"
#include <config.h>
#include <rijndael.h>

namespace pals::prim {

CryptoPP::ECB_Mode<CryptoPP::AES>::Decryption aes_ecb_dec(const bytestr::ByteStr &key);

bytestr::ByteStr transform(const bytestr::ByteStr &input, auto tf) {
    bytestr::ByteStr rv{};
    auto sink = new CryptoPP::StringSinkTemplate<std::vector<uint8_t>>(rv.data);
    auto filter = new CryptoPP::StreamTransformationFilter(tf, sink);
    CryptoPP::StringSource ss(input.as_bytearr(), input.size(), true, filter);
    return rv;
}

} // namespace pals::prim
