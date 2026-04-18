#include "AuthController.h"
#include "utils/OAuthUtils.h"
#include "utils/JwtUtils.h"
#include <drogon/drogon.h>
#include <json/json.h>
#include <trantor/utils/Logger.h>

// Helper: wrap a move-only callback in a shared_ptr so it can be
// captured by multiple nested lambdas without double-move issues.
template<typename F>
auto shared_cb(F&& f) {
    return std::make_shared<std::decay_t<F>>(std::forward<F>(f));
}

void AuthController::googleLogin(const drogon::HttpRequestPtr& req,
                                  std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    callback(drogon::HttpResponse::newRedirectionResponse(OAuthUtils::getAuthUrl()));
}

void AuthController::googleCallback(const drogon::HttpRequestPtr& req,
                                     std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    auto code = req->getParameter("code");
    if (code.empty()) {
        Json::Value err;
        err["error"] = "Missing authorization code";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    auto cb = shared_cb(std::move(callback));

    OAuthUtils::getUserInfo(code, [cb](std::optional<GoogleUserInfo> userInfo) {
        if (!userInfo) {
            Json::Value err;
            err["error"] = "Failed to authenticate with Google";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(drogon::k502BadGateway);
            (*cb)(resp);
            return;
        }

        auto db = drogon::app().getDbClient();
        db->execSqlAsync(
            // Upsert: insert or update name/picture on conflict, then return id.
            "INSERT INTO users (google_id, email, name, picture) VALUES (?, ?, ?, ?)"
            " ON CONFLICT(google_id) DO UPDATE SET"
            "   name       = excluded.name,"
            "   picture    = excluded.picture,"
            "   updated_at = unixepoch()"
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
                auto token      = JwtUtils::createToken(userId);
                auto frontendUrl = drogon::app().getCustomConfig()["frontend_url"].asString();
                // Redirect to the frontend's auth callback with the JWT.
                // The frontend stores it and sends it as Bearer on subsequent requests.
                (*cb)(drogon::HttpResponse::newRedirectionResponse(
                    frontendUrl + "/auth/callback?token=" + token));
            },
            [cb](const drogon::orm::DrogonDbException& e) {
                LOG_ERROR << "DB error in googleCallback: " << e.base().what();
                Json::Value err;
                err["error"] = "Database error";
                auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
                resp->setStatusCode(drogon::k500InternalServerError);
                (*cb)(resp);
            },
            userInfo->googleId, userInfo->email, userInfo->name, userInfo->picture
        );
    });
}

void AuthController::getMe(const drogon::HttpRequestPtr& req,
                            std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    auto userId = req->attributes()->get<int64_t>("userId");
    auto cb     = shared_cb(std::move(callback));
    auto db     = drogon::app().getDbClient();

    db->execSqlAsync(
        "SELECT id, email, name, picture, bio, age, gender, interested_in, created_at"
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
            user["id"]    = row["id"].as<int64_t>();
            user["email"] = row["email"].as<std::string>();
            user["name"]  = row["name"].as<std::string>();
            user["picture"]       = row["picture"].isNull()       ? "" : row["picture"].as<std::string>();
            user["bio"]           = row["bio"].isNull()           ? "" : row["bio"].as<std::string>();
            if (!row["age"].isNull())           user["age"]           = row["age"].as<int>();
            if (!row["gender"].isNull())        user["gender"]        = row["gender"].as<std::string>();
            if (!row["interested_in"].isNull()) user["interested_in"] = row["interested_in"].as<std::string>();
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
