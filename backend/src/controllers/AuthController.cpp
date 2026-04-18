#include "AuthController.h"
#include "utils/JwtUtils.h"
// #include "utils/OAuthUtils.h"   // Google OAuth — re-enable with routes below
#include <drogon/drogon.h>
#include <json/json.h>
#include <trantor/utils/Logger.h>

template<typename F>
auto shared_cb(F&& f) {
    return std::make_shared<std::decay_t<F>>(std::forward<F>(f));
}

void AuthController::login(const drogon::HttpRequestPtr& req,
                            std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    auto body = req->getJsonObject();

    auto badRequest = [&callback](const char* msg) {
        Json::Value err;
        err["error"] = msg;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
    };

    if (!body)                                        { badRequest("Invalid JSON body");      return; }
    if (!body->isMember("phone_number") || (*body)["phone_number"].asString().empty())
                                                      { badRequest("phone_number is required"); return; }
    if (!body->isMember("password") || (*body)["password"].asString().empty())
                                                      { badRequest("password is required");   return; }

    std::string phone = (*body)["phone_number"].asString();

    auto cb = shared_cb(std::move(callback));
    auto db = drogon::app().getDbClient();

    // Find or create a user by phone number — password is not validated (fake auth).
    db->execSqlAsync(
        "INSERT INTO users (phone_number) VALUES (?)"
        " ON CONFLICT(phone_number) DO UPDATE SET updated_at=unixepoch()"
        " RETURNING id",
        [cb](const drogon::orm::Result& r) {
            if (r.empty()) {
                Json::Value err;
                err["error"] = "Database error";
                auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
                resp->setStatusCode(drogon::k500InternalServerError);
                (*cb)(resp);
                return;
            }
            int64_t userId = r[0]["id"].as<int64_t>();
            Json::Value resp;
            resp["token"] = JwtUtils::createToken(userId);
            (*cb)(drogon::HttpResponse::newHttpJsonResponse(resp));
        },
        [cb](const drogon::orm::DrogonDbException& e) {
            LOG_ERROR << "DB error in login: " << e.base().what();
            Json::Value err;
            err["error"] = "Database error";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(drogon::k500InternalServerError);
            (*cb)(resp);
        },
        phone
    );
}

void AuthController::getMe(const drogon::HttpRequestPtr& req,
                            std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    auto userId = req->attributes()->get<int64_t>("userId");
    auto cb     = shared_cb(std::move(callback));
    auto db     = drogon::app().getDbClient();

    db->execSqlAsync(
        "SELECT id, name, email, picture, age, location, phone_number, enrichment_status, created_at"
        " FROM users WHERE id = ?",
        [cb](const drogon::orm::Result& r) {
            if (r.empty()) {
                Json::Value err;
                err["error"] = "User not found";
                auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
                resp->setStatusCode(drogon::k404NotFound);
                (*cb)(resp);
                return;
            }
            const auto& row = r[0];
            Json::Value user;
            user["id"]                = row["id"].as<int64_t>();
            user["enrichment_status"] = row["enrichment_status"].as<std::string>();
            if (!row["name"].isNull())         user["name"]         = row["name"].as<std::string>();
            if (!row["email"].isNull())        user["email"]        = row["email"].as<std::string>();
            if (!row["picture"].isNull())      user["picture"]      = row["picture"].as<std::string>();
            if (!row["age"].isNull())          user["age"]          = row["age"].as<int>();
            if (!row["location"].isNull())     user["location"]     = row["location"].as<std::string>();
            if (!row["phone_number"].isNull()) user["phone_number"] = row["phone_number"].as<std::string>();
            user["created_at"] = row["created_at"].as<int64_t>();
            (*cb)(drogon::HttpResponse::newHttpJsonResponse(user));
        },
        [cb](const drogon::orm::DrogonDbException& e) {
            LOG_ERROR << "DB error in getMe: " << e.base().what();
            Json::Value err;
            err["error"] = "Database error";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(drogon::k500InternalServerError);
            (*cb)(resp);
        },
        userId
    );
}

// =============================================================================
// Google OAuth handlers — commented out, keep for future re-enablement.
// To restore:
//   1. Uncomment routes in AuthController.h
//   2. Uncomment #include "utils/OAuthUtils.h" above
//   3. Uncomment OAuthUtils::init(...) in main.cpp
//   4. Add google_client_id / google_client_secret / google_redirect_uri to config.json
// =============================================================================

// void AuthController::googleLogin(const drogon::HttpRequestPtr& req,
//                                   std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
//     callback(drogon::HttpResponse::newRedirectionResponse(OAuthUtils::getAuthUrl()));
// }

// void AuthController::googleCallback(const drogon::HttpRequestPtr& req,
//                                      std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
//     auto code = req->getParameter("code");
//     if (code.empty()) {
//         Json::Value err; err["error"] = "Missing authorization code";
//         auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
//         resp->setStatusCode(drogon::k400BadRequest);
//         callback(resp); return;
//     }
//     auto cb = shared_cb(std::move(callback));
//     OAuthUtils::getUserInfo(code, [cb](std::optional<GoogleUserInfo> userInfo) {
//         if (!userInfo) {
//             Json::Value err; err["error"] = "Failed to authenticate with Google";
//             auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
//             resp->setStatusCode(drogon::k502BadGateway);
//             (*cb)(resp); return;
//         }
//         auto db = drogon::app().getDbClient();
//         db->execSqlAsync(
//             "INSERT INTO users (google_id, email, name, picture) VALUES (?, ?, ?, ?)"
//             " ON CONFLICT(google_id) DO UPDATE SET"
//             "   name=excluded.name, picture=excluded.picture, updated_at=unixepoch()"
//             " RETURNING id",
//             [cb](const drogon::orm::Result& r) {
//                 int64_t userId = r[0]["id"].as<int64_t>();
//                 auto token = JwtUtils::createToken(userId);
//                 auto frontendUrl = drogon::app().getCustomConfig()["frontend_url"].asString();
//                 (*cb)(drogon::HttpResponse::newRedirectionResponse(
//                     frontendUrl + "/auth/callback?token=" + token));
//             },
//             [cb](const drogon::orm::DrogonDbException& e) {
//                 LOG_ERROR << "DB error in googleCallback: " << e.base().what();
//                 Json::Value err; err["error"] = "Database error";
//                 auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
//                 resp->setStatusCode(drogon::k500InternalServerError);
//                 (*cb)(resp);
//             },
//             userInfo->googleId, userInfo->email, userInfo->name, userInfo->picture
//         );
//     });
// }
