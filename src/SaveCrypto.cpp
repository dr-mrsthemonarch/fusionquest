#include "../include/SaveCrypto.h"
#include "../include/Sha256.h"
#include <cstring>
#include <random>

namespace {
// master key is stored scrambled and rebuilt at runtime. extractable by a determined
// reverse engineer; stops casual save editing only.
const uint8_t kScrambled[32] = {30,35,65,65,250,241,204,80,239,85,239,181,149,0,199,189,108,7,115,202,13,97,253,230,233,153,3,229,61,52,174,89};
const char kMagic[4] = {'F', 'Q', 'S', '1'};
constexpr size_t kNonce = 16, kTag = 32, kHeader = 4 + kNonce + 8;

std::vector<uint8_t> derive(const char* label) {
    std::vector<uint8_t> in(32);
    for (size_t i = 0; i < 32; ++i) in[i] = static_cast<uint8_t>(kScrambled[i] ^ ((0x3C + i * 7) & 0xFF));
    in.insert(in.end(), label, label + std::strlen(label));
    auto d = sha::sha256(in);
    return std::vector<uint8_t>(d.begin(), d.end());
}

// counter-mode keystream from sha256(encKey || nonce || counter)
void applyStream(const std::vector<uint8_t>& encKey, const uint8_t* nonce, uint8_t* buf, size_t len) {
    for (size_t off = 0, ctr = 0; off < len; ++ctr) {
        std::vector<uint8_t> in(encKey);
        in.insert(in.end(), nonce, nonce + kNonce);
        for (int i = 0; i < 8; ++i) in.push_back(static_cast<uint8_t>(uint64_t(ctr) >> (i * 8)));
        auto block = sha::sha256(in);
        for (size_t i = 0; i < 32 && off < len; ++i, ++off) buf[off] ^= block[i];
    }
}
}  // namespace

std::string SaveCrypto::seal(const std::string& plain) {
    std::random_device rd;
    uint8_t nonce[kNonce];
    for (auto& b : nonce) b = static_cast<uint8_t>(rd());

    std::string out(kMagic, 4);
    out.append(reinterpret_cast<const char*>(nonce), kNonce);
    const uint64_t len = plain.size();
    for (int i = 0; i < 8; ++i) out.push_back(static_cast<char>(len >> (i * 8)));

    std::string cipher = plain;
    applyStream(derive("fq-enc"), nonce, reinterpret_cast<uint8_t*>(cipher.data()), cipher.size());
    out += cipher;

    auto tag = sha::hmacSha256(derive("fq-mac"), reinterpret_cast<const uint8_t*>(out.data()), out.size());
    out.append(reinterpret_cast<const char*>(tag.data()), tag.size());
    return out;
}

bool SaveCrypto::isSealed(const std::string& file) {
    return file.size() >= 4 && std::memcmp(file.data(), kMagic, 4) == 0;
}

bool SaveCrypto::open(const std::string& file, std::string& plain) {
    if (file.size() < kHeader + kTag || !isSealed(file)) return false;
    uint64_t len = 0;
    for (int i = 0; i < 8; ++i) len |= uint64_t(static_cast<uint8_t>(file[4 + kNonce + i])) << (i * 8);
    if (len != file.size() - kHeader - kTag) return false;

    const size_t bodyLen = file.size() - kTag;
    auto tag = sha::hmacSha256(derive("fq-mac"), reinterpret_cast<const uint8_t*>(file.data()), bodyLen);
    if (!sha::constantTimeEqual(tag.data(), reinterpret_cast<const uint8_t*>(file.data()) + bodyLen, kTag)) return false;

    plain.assign(file, kHeader, len);
    applyStream(derive("fq-enc"), reinterpret_cast<const uint8_t*>(file.data()) + 4,
                reinterpret_cast<uint8_t*>(plain.data()), plain.size());
    return true;
}
