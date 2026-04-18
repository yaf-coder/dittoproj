#pragma once
#include <functional>
#include <optional>
#include <string>

struct GoogleUserInfo {
    std::string googleId;
    std::string email;
    std::string name;
    std::string picture;
};

class OAuthUtils {
public:
    static void init(const std::string& clientId,
                     const std::string& clientSecret,
                     const std::string& redirectUri);

    // Returns the Google OAuth2 authorization URL to redirect the user to.
    static std::string getAuthUrl(const std::string& state = "");

    // Exchanges an authorization code for user info. Fully async — uses
    // Drogon's built-in HTTP client so it never blocks the event loop.
    static void getUserInfo(
        const std::string& code,
        std::function<void(std::optional<GoogleUserInfo>)> callback
    );

private:
    static inline std::string clientId_;
    static inline std::string clientSecret_;
    static inline std::string redirectUri_;
};
