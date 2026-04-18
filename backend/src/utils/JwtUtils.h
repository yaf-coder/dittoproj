#pragma once
#include <cstdint>
#include <optional>
#include <string>

class JwtUtils {
public:
    static void init(const std::string& secret);
    static std::string createToken(int64_t userId);
    static std::optional<int64_t> validateToken(const std::string& token);

private:
    static inline std::string secret_;
};
