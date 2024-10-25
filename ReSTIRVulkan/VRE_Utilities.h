#pragma once

#include <functional>

namespace VRE {
    // Stack Overflow. (2010). How do I combine hash values in C++0x? [online] Available at: https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x/57595105#57595105.
    template <typename T, typename... Rest>
    void hashCombine(std::size_t& seed, const T& v, const Rest&... rest) {
        seed ^= std::hash<T>{}(v)+0x9e3779b9 + (seed << 6) + (seed >> 2);
        (hashCombine(seed, rest), ...);
    };
}