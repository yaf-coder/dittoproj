#pragma once
#include <string>

// SHA-256 password hashing via OpenSSL EVP.
// Not bcrypt, but sufficient for a fake/demo auth system.
class PasswordUtils {
public:
    static std::string hash(const std::string& password);
    static bool verify(const std::string& password, const std::string& hash);
};
