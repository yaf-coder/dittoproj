#include "AuthController.h"
#include "utils/AuthUtils.h"
#include "utils/JwtUtils.h"
#include <drogon/drogon.h>
#include <json/json.h>
#include <trantor/utils/Logger.h>

template<typename F>
auto shared_cb(F&& f) {
    return std::make_shared<std::decay_t<F>>(std::forward<F>(f));
}

void AuthController::registerUser(const drogon::HttpRequestPtr& req,
                                   std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    auto body = req->getJsonObject();

    auto badRequest = [&callback](const char* msg) {
        Json::Value err;
        err["error"] = msg;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
    };

    if (!body) { badRequest("Invalid JSON body"); return; }

    std::string phone    = (*body).get("phone_number", "").asString();
    std::string password = (*body).get("password",     "").asString();
    std::string name     = (*body).get("name",         "").asString();

    if (phone.empty())         { badRequest("phone_number is required"); return; }
    if (password.size() < 8)   { badRequest("password must be at least 8 characters"); return; }
    if (name.empty())          { badRequest("name is required"); return; }

    std::string hash;
    try {
        hash = AuthUtils::hashPassword(password);
    } catch (const std::exception& e) {
        LOG_ERROR << "hashPassword failed: " << e.what();
        Json::Value err;
        err["error"] = "Internal error";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k500InternalServerError);
        callback(resp);
        return;
    }

    auto cb = shared_cb(std::move(callback));
    auto db = drogon::app().getDbClient();

    // Insert the user, then fetch the new ID with a follow-up query.
    // (Avoids RETURNING which some SQLite ORM versions handle inconsistently.)
    db->execSqlAsync(
        "INSERT INTO users (phone_number, password_hash, name) VALUES (?, ?, ?)",
        [cb, db, name, phone](const drogon::orm::Result&) {
            db->execSqlAsync(
                "SELECT last_insert_rowid() AS id",
                [cb, name, phone](const drogon::orm::Result& r) {
                    if (r.empty()) {
                        Json::Value err;
                        err["error"] = "Registration failed";
                        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
                        resp->setStatusCode(drogon::k500InternalServerError);
                        (*cb)(resp);
                        return;
                    }
                    int64_t userId = r[0]["id"].as<int64_t>();
                    Json::Value resp;
                    resp["token"]        = JwtUtils::createToken(userId);
                    resp["id"]           = userId;
                    resp["name"]         = name;
                    resp["phone_number"] = phone;
                    (*cb)(drogon::HttpResponse::newHttpJsonResponse(resp));
                },
                [cb](const drogon::orm::DrogonDbException& e) {
                    LOG_ERROR << "DB error fetching new user id: " << e.base().what();
                    Json::Value err;
                    err["error"] = "Database error";
                    auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
                    resp->setStatusCode(drogon::k500InternalServerError);
                    (*cb)(resp);
                }
            );
        },
        [cb](const drogon::orm::DrogonDbException& e) {
            std::string what = e.base().what();
            LOG_ERROR << "DB error in registerUser: " << what;
            Json::Value err;
            if (what.find("UNIQUE") != std::string::npos) {
                err["error"] = "Phone number already registered";
                auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
                resp->setStatusCode(drogon::k409Conflict);
                (*cb)(resp);
            } else {
                err["error"] = "Database error";
                auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
                resp->setStatusCode(drogon::k500InternalServerError);
                (*cb)(resp);
            }
        },
        phone, hash, name
    );
}

void AuthController::loginUser(const drogon::HttpRequestPtr& req,
                                std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    auto body = req->getJsonObject();

    auto badRequest = [&callback](const char* msg) {
        Json::Value err;
        err["error"] = msg;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
    };

    if (!body) { badRequest("Invalid JSON body"); return; }

    std::string phone    = (*body).get("phone_number", "").asString();
    std::string password = (*body).get("password",     "").asString();

    if (phone.empty())    { badRequest("phone_number is required"); return; }
    if (password.empty()) { badRequest("password is required");     return; }

    auto cb = shared_cb(std::move(callback));
    auto db = drogon::app().getDbClient();

    db->execSqlAsync(
        "SELECT id, name, password_hash FROM users WHERE phone_number = ?",
        [cb, password](const drogon::orm::Result& r) {
            // Use the same error message for not-found and wrong password
            // to avoid leaking whether a phone number is registered.
            auto unauthorized = [&cb]() {
                Json::Value err;
                err["error"] = "Invalid phone number or password";
                auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
                resp->setStatusCode(drogon::k401Unauthorized);
                (*cb)(resp);
            };

            if (r.empty()) { unauthorized(); return; }

            const auto& row  = r[0];
            std::string hash = row["password_hash"].as<std::string>();

            if (!AuthUtils::verifyPassword(password, hash)) {
                unauthorized();
                return;
            }

            int64_t userId = row["id"].as<int64_t>();
            Json::Value resp;
            resp["token"] = JwtUtils::createToken(userId);
            resp["id"]    = userId;
            resp["name"]  = row["name"].as<std::string>();
            (*cb)(drogon::HttpResponse::newHttpJsonResponse(resp));
        },
        [cb](const drogon::orm::DrogonDbException& e) {
            LOG_ERROR << "DB error in loginUser: " << e.base().what();
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
        "SELECT id, phone_number, name, age, location,"
        "       gender, ethnicity, education_level, political_belief,"
        "       picture, created_at"
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
            user["id"]           = row["id"].as<int64_t>();
            user["phone_number"] = row["phone_number"].as<std::string>();
            user["name"]         = row["name"].as<std::string>();
            user["picture"]      = row["picture"].isNull() ? "" : row["picture"].as<std::string>();
            user["created_at"]   = row["created_at"].as<int64_t>();
            if (!row["age"].isNull())              user["age"]              = row["age"].as<int>();
            if (!row["location"].isNull())         user["location"]         = row["location"].as<std::string>();
            if (!row["gender"].isNull())           user["gender"]           = row["gender"].as<std::string>();
            if (!row["ethnicity"].isNull())        user["ethnicity"]        = row["ethnicity"].as<std::string>();
            if (!row["education_level"].isNull())  user["education_level"]  = row["education_level"].as<std::string>();
            if (!row["political_belief"].isNull()) user["political_belief"] = row["political_belief"].as<float>();
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
