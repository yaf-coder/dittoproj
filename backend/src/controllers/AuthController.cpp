#include "AuthController.h"
#include "utils/JwtUtils.h"
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

    if (!body)                                  { badRequest("Invalid JSON body"); return; }
    if (!body->isMember("name") || (*body)["name"].asString().empty())
                                                { badRequest("name is required");  return; }
    if (!body->isMember("password") || (*body)["password"].asString().empty())
                                                { badRequest("password is required"); return; }

    std::string name = (*body)["name"].asString();

    auto cb = shared_cb(std::move(callback));
    auto db = drogon::app().getDbClient();

    // Find or create a user by name — password is not validated (fake auth).
    db->execSqlAsync(
        "INSERT INTO users (name) VALUES (?)"
        " ON CONFLICT(name) DO UPDATE SET updated_at=unixepoch()"
        " RETURNING id",
        [cb, name](const drogon::orm::Result& r) {
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
        name
    );
}

void AuthController::getMe(const drogon::HttpRequestPtr& req,
                            std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    auto userId = req->attributes()->get<int64_t>("userId");
    auto cb     = shared_cb(std::move(callback));
    auto db     = drogon::app().getDbClient();

    db->execSqlAsync(
        "SELECT id, name, email, picture, age, location, enrichment_status, created_at"
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
            user["name"]              = row["name"].as<std::string>();
            user["enrichment_status"] = row["enrichment_status"].as<std::string>();
            if (!row["email"].isNull())    user["email"]    = row["email"].as<std::string>();
            if (!row["picture"].isNull())  user["picture"]  = row["picture"].as<std::string>();
            if (!row["age"].isNull())      user["age"]      = row["age"].as<int>();
            if (!row["location"].isNull()) user["location"] = row["location"].as<std::string>();
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
