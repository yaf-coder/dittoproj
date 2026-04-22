#include "AuthUtils.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <vector>

static constexpr int SALT_BYTES    = 16;
static constexpr int HASH_BYTES    = 32;
static constexpr int ITERATIONS    = 100000;

static std::string toHex(const unsigned char* data, size_t len) {
    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < len; ++i)
        ss << std::setw(2) << static_cast<int>(data[i]);
    return ss.str();
}

static std::vector<unsigned char> fromHex(const std::string& hex) {
    std::vector<unsigned char> out;
    out.reserve(hex.size() / 2);
    for (size_t i = 0; i + 1 < hex.size(); i += 2) {
        unsigned int byte;
        std::istringstream(hex.substr(i, 2)) >> std::hex >> byte;
        out.push_back(static_cast<unsigned char>(byte));
    }
    return out;
}

std::string AuthUtils::hashPassword(const std::string& password) {
    unsigned char salt[SALT_BYTES];
    if (RAND_bytes(salt, SALT_BYTES) != 1)
        throw std::runtime_error("Failed to generate random salt");

    unsigned char hash[HASH_BYTES];
    if (PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.size()),
                           salt, SALT_BYTES, ITERATIONS,
                           EVP_sha256(), HASH_BYTES, hash) != 1)
        throw std::runtime_error("PBKDF2 failed");

    return toHex(salt, SALT_BYTES) + ":" + toHex(hash, HASH_BYTES);
}

bool AuthUtils::verifyPassword(const std::string& password, const std::string& stored) {
    auto sep = stored.find(':');
    if (sep == std::string::npos) return false;

    auto saltVec = fromHex(stored.substr(0, sep));
    auto hashVec = fromHex(stored.substr(sep + 1));
    if (saltVec.size() != SALT_BYTES || hashVec.size() != HASH_BYTES) return false;

    unsigned char computed[HASH_BYTES];
    if (PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.size()),
                           saltVec.data(), SALT_BYTES, ITERATIONS,
                           EVP_sha256(), HASH_BYTES, computed) != 1)
        return false;

    // Constant-time compare to prevent timing attacks.
    unsigned char diff = 0;
    for (int i = 0; i < HASH_BYTES; ++i)
        diff |= computed[i] ^ hashVec[i];
    return diff == 0;
}
