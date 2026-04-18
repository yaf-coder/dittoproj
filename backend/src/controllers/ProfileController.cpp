#include "ProfileController.h"
#include "services/EnrichmentService.h"
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
        "SELECT id, name, age, location,"
        "       projected_gender, projected_ethnicity,"
        "       education_level, political_belief,"
        "       enrichment_status, picture"
        " FROM users WHERE id = ?",
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

            // Collected fields
            profile["id"]       = row["id"].as<int64_t>();
            profile["name"]     = row["name"].as<std::string>();
            profile["picture"]  = row["picture"].isNull() ? "" : row["picture"].as<std::string>();
            if (!row["age"].isNull())      profile["age"]      = row["age"].as<int>();
            if (!row["location"].isNull()) profile["location"] = row["location"].as<std::string>();

            // Projected fields
            profile["enrichment_status"] = row["enrichment_status"].as<std::string>();
            if (!row["projected_gender"].isNull())
                profile["projected_gender"]   = row["projected_gender"].as<std::string>();
            if (!row["projected_ethnicity"].isNull())
                profile["projected_ethnicity"] = row["projected_ethnicity"].as<std::string>();
            if (!row["education_level"].isNull())
                profile["education_level"]     = row["education_level"].as<std::string>();
            if (!row["political_belief"].isNull())
                profile["political_belief"]    = row["political_belief"].as<float>();

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

    if (!body) { badRequest("Invalid JSON body"); return; }

    // Only these four fields are collected from the user.
    std::string name        = (*body).get("name",         "").asString();
    int         age         = (*body).get("age",           0).asInt();
    std::string location    = (*body).get("location",     "").asString();
    std::string phoneNumber = (*body).get("phone_number", "").asString();

    if (name.empty())           { badRequest("name is required");                  return; }
    if (age < 18 || age > 120)  { badRequest("age must be between 18 and 120");    return; }
    if (location.empty())       { badRequest("location is required");              return; }

    auto cb = shared_cb(std::move(callback));
    auto db = drogon::app().getDbClient();

    db->execSqlAsync(
        "UPDATE users"
        " SET name=?, age=?, location=?, phone_number=?,"
        "     enrichment_status='pending', updated_at=unixepoch()"
        " WHERE id=?",
        [cb, db, userId, name, age, location](const drogon::orm::Result&) {
            // Respond immediately — enrichment runs in the background.
            Json::Value resp;
            resp["success"]            = true;
            resp["enrichment_status"]  = "pending";
            (*cb)(drogon::HttpResponse::newHttpJsonResponse(resp));

            // Fire-and-forget enrichment pipeline.
            EnrichmentService::enrich(userId, name, age, location, db);
        },
        [cb](const drogon::orm::DrogonDbException& e) {
            LOG_ERROR << "DB error in updateProfile: " << e.base().what();
            Json::Value err;
            err["error"] = "Database error";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(drogon::k500InternalServerError);
            (*cb)(resp);
        },
        name, age, location, phoneNumber, userId
    );
}
