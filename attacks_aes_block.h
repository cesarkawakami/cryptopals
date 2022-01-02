#include "ByteStr.h"
#include "range/v3/view/chunk.hpp"
#include "range/v3/view/repeat_n.hpp"
#include <set>

namespace pals::attacks_aes_block {

enum struct EcbOrCbc {
    ECB = 1,
    CBC = 2,
};

EcbOrCbc detect_from_encryption_oracle(auto oracle) {
    const auto &oracle_input = bytestr::ByteStr{ranges::views::repeat_n(uint8_t{0}, 15 + 16 + 16) |
                                                ranges::to<std::vector>()};
    const bytestr::ByteStr &oracle_output = oracle(oracle_input);
    std::set<bytestr::ByteStr> seen;
    for (const auto &rng : oracle_output | ranges::views::chunk(16)) {
        const auto &chunk = bytestr::ByteStr{rng | ranges::to<std::vector>()};
        if (seen.contains(chunk)) {
            return EcbOrCbc::ECB;
        }
        seen.insert(chunk);
    }
    return EcbOrCbc::CBC;
}

} // namespace pals::attacks_aes_block
