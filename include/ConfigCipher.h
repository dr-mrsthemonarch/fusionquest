#ifndef CONFIGCIPHER_H
#define CONFIGCIPHER_H
#include <cstddef>

// light obfuscation for embedded config (keeps `strings`/hex editors from showing plain text).
// shared by tools/embed_config.cpp (encode) and ConfigStore.cpp (decode). NOT real security.
inline unsigned char configKey(size_t i, size_t salt) {
    return static_cast<unsigned char>(0xA5u ^ ((i * 131u + 7u + salt * 977u) & 0xFFu) ^ ((i >> 8) & 0xFFu));
}
#endif
