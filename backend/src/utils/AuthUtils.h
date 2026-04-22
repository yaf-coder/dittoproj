#pragma once
#include <string>

// Password hashing and verification using PBKDF2-HMAC-SHA256 (via OpenSSL).
// Stored format: "<salt_hex>:<hash_hex>"
class AuthUtils {
public:
    static std::string hashPassword(const std::string& password);
    static bool verifyPassword(const std::string& password, const std::string& stored);
};
