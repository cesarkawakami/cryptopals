#include "utils.h"

namespace pals::utils {

[[noreturn]] void fatal(const std::string &s) {
    std::cerr << s << "\n";
    abort();
}

void expect(bool condition, const std::string &msg) {
    if (!condition) {
        fatal(msg);
    }
}

std::string read_file(const std::string &path) {
    std::ifstream fin{path};
    if (fin.fail()) {
        fatal("could not find file");
    }
    std::ostringstream ss;
    ss << fin.rdbuf();
    return ss.str();
}

} // namespace pals::utils
