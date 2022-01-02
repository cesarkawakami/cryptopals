#include "prim.h"
#include <rijndael.h>

namespace pals::prim {

CryptoPP::ECB_Mode<CryptoPP::AES>::Decryption aes_ecb_dec(const bytestr::ByteStr &key) {
    CryptoPP::ECB_Mode<CryptoPP::AES>::Decryption d;
    d.SetKey(key.as_bytearr(), key.size());
    return d;
}

}
