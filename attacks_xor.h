#include "ByteStr.h"
#include "scoring.h"
#include <limits>
#include <tuple>

namespace pals::attacks_xor {

std::tuple<uint8_t, double> solve_xor1(const pals::bytestr::ByteStr &bs);
std::tuple<uint8_t, double> solve_xor1_simple(const pals::bytestr::ByteStr &bs);

} // namespace pals::attacks_xor
