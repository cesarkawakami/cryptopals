#include <iostream>
#include <range/v3/all.hpp>

namespace R = ranges::views;

int main() {
    for (int i : R::iota(6, 10)) {
        std::cout << i << "\n";
    }
}
