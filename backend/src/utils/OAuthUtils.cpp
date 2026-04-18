#include "OAuthUtils.h"
#include <drogon/HttpClient.h>
#include <drogon/HttpRequest.h>
#include <json/json.h>
#include <jwt-cpp/jwt.h>
#include <trantor/utils/Logger.h>

void OAuthUtils::init(const std::string& clientId,
                       const std::string& clientSecret,
                       const std::string& redirectUri) {
    clientId_    = clientId;
    clientSecret_ = clientSecret;
    redirectUri_  = redirectUri;
}

std::string OAuthUtils::getAuthUrl(const std::string& state) {
    std::string url =
        "https://accounts.google.com/o/oauth2/v2/auth"
        "?client_id="      + clientId_ +
        "&redirect_uri="   + redirectUri_ +
        "&response_type=code"
        "&scope=openid%20email%20profile"
        "&access_type=offline";
    if (!state.empty()) {
        url += "&state=" + state;
    }
    return url;
}

void OAuthUtils::getUserInfo(const std::string& code,
                              std::function<void(std::optional<GoogleUserInfo>)> callback) {
    // Exchange authorization code for tokens via Google's token endpoint.
    // Drogon's async HTTP client is used so this never blocks the event loop.
    auto client = drogon::HttpClient::newHttpClient("https://oauth2.googleapis.com");
    auto req    = drogon::HttpRequest::newHttpRequest();
    req->setMethod(drogon::Post);
    req->setPath("/token");
    req->addHeader("Content-Type", "application/x-www-form-urlencoded");
    req->setBody(
        "code="          + code          +
        "&client_id="    + clientId_     +
        "&client_secret=" + clientSecret_ +
        "&redirect_uri=" + redirectUri_  +
        "&grant_type=authorization_code"
    );

    client->sendRequest(req,
        [cb = std::move(callback)](drogon::ReqResult result,
                                   const drogon::HttpResponsePtr& resp) mutable {
            if (result != drogon::ReqResult::Ok || !resp ||
                resp->statusCode() != drogon::k200OK) {
                LOG_WARN << "Google token exchange failed, result=" << static_cast<int>(result);
                cb(std::nullopt);
                return;
            }

            Json::Value json;
            Json::Reader reader;
            if (!reader.parse(resp->body(), json)) {
                LOG_WARN << "Failed to parse Google token response";
                cb(std::nullopt);
                return;
            }

            auto idTokenStr = json["id_token"].asString();
            if (idTokenStr.empty()) {
                LOG_WARN << "No id_token in Google response";
                cb(std::nullopt);
                return;
            }

            // Decode Google's id_token (a JWT) to extract user claims.
            // Note: for production, verify the signature against Google's JWKS endpoint.
            try {
                auto decoded = jwt::decode(idTokenStr);
                GoogleUserInfo info;
                info.googleId = decoded.get_subject();
                info.email    = decoded.get_payload_claim("email").as_string();
                info.name     = decoded.get_payload_claim("name").as_string();
                if (decoded.has_payload_claim("picture")) {
                    info.picture = decoded.get_payload_claim("picture").as_string();
                }
                cb(info);
            } catch (const std::exception& e) {
                LOG_WARN << "Failed to decode Google id_token: " << e.what();
                cb(std::nullopt);
            }
        }
    );
}
