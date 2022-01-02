#include "prim.h"
#include "ByteStr.h"
#include "range/v3/algorithm/copy.hpp"
#include "range/v3/view/repeat_n.hpp"
#include <iterator>
#include <rijndael.h>

namespace R = ranges::views;

namespace pals::prim {

CryptoPP::ECB_Mode<CryptoPP::AES>::Decryption aes_ecb_dec(const bytestr::ByteStr &key) {
    CryptoPP::ECB_Mode<CryptoPP::AES>::Decryption d;
    d.SetKey(key.as_bytearr(), key.size());
    return d;
}

bytestr::ByteStr &pkcs7_i(bytestr::ByteStr &bs, int block_size) {
    int bytes_to_add = block_size - (bs.size() % block_size);
    ranges::copy(R::repeat_n(static_cast<uint8_t>(bytes_to_add), bytes_to_add),
                 std::back_inserter(bs.data));
    return bs;
}

bytestr::ByteStr pkcs7(const bytestr::ByteStr &bs, int block_size) {
    bytestr::ByteStr rv{bs};
    return pkcs7_i(rv, block_size);
}

} // namespace pals::prim
