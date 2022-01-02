#include "prim.h"
#include "ByteStr.h"
#include "aes.h"
#include "modes.h"
#include "range/v3/algorithm/any_of.hpp"
#include "range/v3/algorithm/copy.hpp"
#include "range/v3/view/chunk.hpp"
#include "range/v3/view/repeat_n.hpp"
#include "range/v3/view/reverse.hpp"
#include "utils.h"
#include <iterator>
#include <rijndael.h>

namespace R = ranges::views;

namespace pals::prim {

bytestr::ByteStr aes_ecb_enc(const bytestr::ByteStr &clear_text, const bytestr::ByteStr &key) {
    CryptoPP::ECB_Mode<CryptoPP::AES>::Encryption e;
    e.SetKey(key.as_bytearr(), key.size());
    return transform(clear_text, e, _CryPaddingScheme::NO_PADDING);
}
bytestr::ByteStr aes_ecb_dec(const bytestr::ByteStr &cipher_text, const bytestr::ByteStr &key) {
    CryptoPP::ECB_Mode<CryptoPP::AES>::Decryption d;
    d.SetKey(key.as_bytearr(), key.size());
    return transform(cipher_text, d, _CryPaddingScheme::NO_PADDING);
}

bytestr::ByteStr &pkcs7_pad_i(bytestr::ByteStr &bs, int block_size) {
    int bytes_to_add = block_size - (bs.size() % block_size);
    ranges::copy(R::repeat_n(static_cast<uint8_t>(bytes_to_add), bytes_to_add),
                 std::back_inserter(bs.data));
    return bs;
}

bytestr::ByteStr pkcs7_pad(const bytestr::ByteStr &bs, int block_size) {
    bytestr::ByteStr rv{bs};
    return pkcs7_pad_i(rv, block_size);
}

bytestr::ByteStr &pkcs7_unpad_i(bytestr::ByteStr &bs) {
    std::size_t bytes_to_remove = bs.data.back();
    utils::expect(bs.size() >= bytes_to_remove, "invalid pkcs7 padding");
    if (ranges::any_of(bs.data | R::reverse | R::take(bytes_to_remove),
                       [&](auto c) { return c != bytes_to_remove; })) {
        utils::fatal("invalid pkcs7 padding");
    }
    bs.data.erase(bs.data.end() - bytes_to_remove, bs.data.end());
    return bs;
}

bytestr::ByteStr pkcs7_unpad(const bytestr::ByteStr &bs) {
    bytestr::ByteStr rv{bs};
    return pkcs7_unpad_i(rv);
}

bytestr::ByteStr aes_cbc_enc(const bytestr::ByteStr &clear_text,
                             const bytestr::ByteStr &key,
                             const bytestr::ByteStr &iv) {
    const auto &padded_clear_text = pkcs7_pad(clear_text, AES_BLOCK_SIZE);
    bytestr::ByteStr prev = iv;
    bytestr::ByteStr rv;
    for (const auto &clear_block_rng : padded_clear_text | R::chunk(16)) {
        const auto &clear_block = bytestr::ByteStr{clear_block_rng | ranges::to<std::vector>()};
        utils::expect(clear_block.size() == 16, "padding failure?");
        const auto &cipher_block = aes_ecb_enc(clear_block ^ prev, key);
        utils::expect(cipher_block.size() == 16, "wtf?");
        ranges::copy(cipher_block, std::back_inserter(rv));
        prev = cipher_block;
    }
    return rv;
}

bytestr::ByteStr aes_cbc_dec(const bytestr::ByteStr &cipher_text,
                             const bytestr::ByteStr &key,
                             const bytestr::ByteStr &iv) {
    utils::expect(cipher_text.size() % 16 == 0, "AES cipher text with weird length");
    bytestr::ByteStr prev = iv;
    bytestr::ByteStr padded_clear_text;
    for (const auto &cipher_block_rng : cipher_text | R::chunk(16)) {
        const auto &cipher_block = bytestr::ByteStr{cipher_block_rng | ranges::to<std::vector>()};
        utils::expect(cipher_block.size() == 16, "??");
        const auto &clear_block = aes_ecb_dec(cipher_block, key) ^ prev;
        ranges::copy(clear_block, std::back_inserter(padded_clear_text));
        prev = cipher_block;
    }
    return pkcs7_unpad(padded_clear_text);
}

} // namespace pals::prim
