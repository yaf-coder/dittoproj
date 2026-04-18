#include "JwtUtils.h"
#include <jwt-cpp/jwt.h>
#include <chrono>
#include <stdexcept>

void JwtUtils::init(const std::string& secret) {
    if (secret.size() < 32) {
        throw std::runtime_error("JWT secret must be at least 32 characters");
    }
    secret_ = secret;
}

std::string JwtUtils::createToken(int64_t userId) {
    using namespace std::chrono;
    return jwt::create()
        .set_issuer("ditto")
        .set_subject(std::to_string(userId))
        .set_issued_at(system_clock::now())
        .set_expires_at(system_clock::now() + hours{24 * 30})
        .sign(jwt::algorithm::hs256{secret_});
}

std::optional<int64_t> JwtUtils::validateToken(const std::string& token) {
    try {
        auto decoded = jwt::decode(token);
        jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{secret_})
            .with_issuer("ditto")
            .verify(decoded);
        return std::stoll(decoded.get_subject());
    } catch (...) {
        return std::nullopt;
    }
}
