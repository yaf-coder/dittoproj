#include "ProfileController.h"
#include <drogon/drogon.h>
#include <json/json.h>
#include <trantor/utils/Logger.h>

template<typename F>
auto shared_cb(F&& f) {
    return std::make_shared<std::decay_t<F>>(std::forward<F>(f));
}

void ProfileController::getProfile(const drogon::HttpRequestPtr& req,
                                    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                    int64_t id) {
    auto cb = shared_cb(std::move(callback));
    auto db = drogon::app().getDbClient();

    db->execSqlAsync(
        "SELECT id, name, picture, bio, age, gender FROM users WHERE id = ?",
        [cb](const drogon::orm::Result& r) {
            if (r.empty()) {
                Json::Value err;
                err["error"] = "Profile not found";
                auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
                resp->setStatusCode(drogon::k404NotFound);
                (*cb)(resp);
                return;
            }
            const auto& row = r[0];
            Json::Value profile;
            profile["id"]      = row["id"].as<int64_t>();
            profile["name"]    = row["name"].as<std::string>();
            profile["picture"] = row["picture"].isNull() ? "" : row["picture"].as<std::string>();
            profile["bio"]     = row["bio"].isNull()     ? "" : row["bio"].as<std::string>();
            if (!row["age"].isNull())    profile["age"]    = row["age"].as<int>();
            if (!row["gender"].isNull()) profile["gender"] = row["gender"].as<std::string>();
            (*cb)(drogon::HttpResponse::newHttpJsonResponse(profile));
        },
        [cb](const drogon::orm::DrogonDbException& e) {
            LOG_ERROR << "DB error in getProfile: " << e.base().what();
            Json::Value err;
            err["error"] = "Database error";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(drogon::k500InternalServerError);
            (*cb)(resp);
        },
        id
    );
}

void ProfileController::updateProfile(const drogon::HttpRequestPtr& req,
                                       std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    auto userId = req->attributes()->get<int64_t>("userId");
    auto body   = req->getJsonObject();

    auto badRequest = [&callback](const char* msg) {
        Json::Value err;
        err["error"] = msg;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
    };

    if (!body) {
        badRequest("Invalid JSON body");
        return;
    }

    std::string bio          = (*body).get("bio",          "").asString();
    int         age          = (*body).get("age",           0).asInt();
    std::string gender       = (*body).get("gender",       "").asString();
    std::string interestedIn = (*body).get("interested_in","").asString();

    if (age < 18 || age > 120) {
        badRequest("age must be between 18 and 120");
        return;
    }

    auto cb = shared_cb(std::move(callback));
    auto db = drogon::app().getDbClient();

    db->execSqlAsync(
        "UPDATE users"
        " SET bio=?, age=?, gender=?, interested_in=?, updated_at=unixepoch()"
        " WHERE id=?",
        [cb](const drogon::orm::Result&) {
            Json::Value resp;
            resp["success"] = true;
            (*cb)(drogon::HttpResponse::newHttpJsonResponse(resp));
        },
        [cb](const drogon::orm::DrogonDbException& e) {
            LOG_ERROR << "DB error in updateProfile: " << e.base().what();
            Json::Value err;
            err["error"] = "Database error";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(drogon::k500InternalServerError);
            (*cb)(resp);
        },
        bio, age, gender, interestedIn, userId
    );
}
