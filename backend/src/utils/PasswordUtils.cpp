#include "PasswordUtils.h"
#include <openssl/evp.h>
#include <iomanip>
#include <sstream>
#include <stdexcept>

std::string PasswordUtils::hash(const std::string& password) {
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int  digestLen = 0;

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) throw std::runtime_error("EVP_MD_CTX_new failed");

    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1 ||
        EVP_DigestUpdate(ctx, password.data(), password.size()) != 1 ||
        EVP_DigestFinal_ex(ctx, digest, &digestLen) != 1) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("SHA-256 digest failed");
    }
    EVP_MD_CTX_free(ctx);

    std::ostringstream ss;
    for (unsigned int i = 0; i < digestLen; ++i)
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(digest[i]);
    return ss.str();
}

bool PasswordUtils::verify(const std::string& password, const std::string& storedHash) {
    return hash(password) == storedHash;
}
