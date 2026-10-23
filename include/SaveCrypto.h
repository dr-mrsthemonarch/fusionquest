#ifndef SAVECRYPTO_H
#define SAVECRYPTO_H

#include <string>

// save container: "FQS1" | nonce16 | len u64 | ciphertext | HMAC-SHA256 tag32 (encrypt-then-MAC).
// edited / truncated / foreign files fail open().
namespace SaveCrypto {
std::string seal(const std::string& plain);
bool open(const std::string& file, std::string& plain);
bool isSealed(const std::string& file);
}

#endif
