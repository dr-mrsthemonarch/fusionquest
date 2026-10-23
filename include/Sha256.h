#ifndef SHA256_H
#define SHA256_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace sha {
using Digest = std::array<uint8_t, 32>;

Digest sha256(const uint8_t* data, size_t len);
Digest sha256(const std::vector<uint8_t>& v);
Digest hmacSha256(const std::vector<uint8_t>& key, const uint8_t* data, size_t len);
bool constantTimeEqual(const uint8_t* a, const uint8_t* b, size_t n);
}  // namespace sha

#endif
