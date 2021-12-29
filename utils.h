#pragma once
#include <iostream>
#include <string>

namespace pals {

[[noreturn]] inline void fatal(const std::string &s) {
    std::cerr << s << "\n";
    abort();
}

inline void assume(bool condition, const std::string &msg) {
    if (!condition) {
        fatal(msg);
    }
}

} // namespace pals
