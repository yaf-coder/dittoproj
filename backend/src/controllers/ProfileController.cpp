#include "ProfileController.h"
#include <drogon/drogon.h>
#include <json/json.h>
#include <trantor/utils/Logger.h>
#include <set>

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
        "       gender, ethnicity, education_level, political_belief,"
        "       picture"
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

            profile["id"]      = row["id"].as<int64_t>();
            profile["name"]    = row["name"].as<std::string>();
            profile["picture"] = row["picture"].isNull() ? "" : row["picture"].as<std::string>();
            if (!row["age"].isNull())              profile["age"]              = row["age"].as<int>();
            if (!row["location"].isNull())         profile["location"]         = row["location"].as<std::string>();
            if (!row["gender"].isNull())           profile["gender"]           = row["gender"].as<std::string>();
            if (!row["ethnicity"].isNull())        profile["ethnicity"]        = row["ethnicity"].as<std::string>();
            if (!row["education_level"].isNull())  profile["education_level"]  = row["education_level"].as<std::string>();
            if (!row["political_belief"].isNull()) profile["political_belief"] = row["political_belief"].as<float>();

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

    std::string name        = (*body).get("name",         "").asString();
    int         age         = (*body).get("age",           0).asInt();
    std::string location    = (*body).get("location",     "").asString();
    std::string phoneNumber = (*body).get("phone_number", "").asString();

    if (name.empty())          { badRequest("name is required");               return; }
    if (age < 18 || age > 120) { badRequest("age must be between 18 and 120"); return; }
    if (location.empty())      { badRequest("location is required");           return; }

    auto cb = shared_cb(std::move(callback));
    auto db = drogon::app().getDbClient();

    db->execSqlAsync(
        "UPDATE users SET name=?, age=?, location=?, phone_number=?, updated_at=unixepoch()"
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
        name, age, location, phoneNumber, userId
    );
}

void ProfileController::enrichProfile(const drogon::HttpRequestPtr& req,
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

    static const std::set<std::string> validGenders = {
        "male", "female", "non_binary", "other", "prefer_not_to_say"
    };
    static const std::set<std::string> validEducation = {
        "high_school", "some_college", "bachelors", "masters", "phd"
    };

    // All fields are optional. We collect everything as strings so we can
    // dispatch execSqlAsync with a simple switch on param count. SQLite's
    // type affinity stores age and political_belief correctly despite the
    // string binding.
    std::string setClauses;
    std::vector<std::string> params; // all values including userId last

    auto addField = [&](const std::string& col, const std::string& val) {
        setClauses += (setClauses.empty() ? "" : ", ") + col + "=?";
        params.push_back(val);
    };

    if (body->isMember("gender")) {
        std::string v = (*body)["gender"].asString();
        if (validGenders.find(v) == validGenders.end()) {
            badRequest("gender must be one of: male, female, non_binary, other, prefer_not_to_say");
            return;
        }
        addField("gender", v);
    }
    if (body->isMember("education_level")) {
        std::string v = (*body)["education_level"].asString();
        if (validEducation.find(v) == validEducation.end()) {
            badRequest("education_level must be one of: high_school, some_college, bachelors, masters, phd");
            return;
        }
        addField("education_level", v);
    }
    if (body->isMember("study_location")) {
        addField("study_location", (*body)["study_location"].asString());
    }
    if (body->isMember("age")) {
        int v = (*body)["age"].asInt();
        if (v < 18 || v > 120) { badRequest("age must be between 18 and 120"); return; }
        addField("age", std::to_string(v));
    }
    if (body->isMember("political_belief")) {
        float v = (*body)["political_belief"].asFloat();
        if (v < 0.0f || v > 1.0f) { badRequest("political_belief must be between 0.0 and 1.0"); return; }
        addField("political_belief", std::to_string(v));
    }

    if (params.empty()) {
        badRequest("No fields provided");
        return;
    }

    setClauses += ", updated_at=unixepoch()";
    std::string sql = "UPDATE users SET " + setClauses + " WHERE id=?";
    params.push_back(std::to_string(userId));

    auto cb = shared_cb(std::move(callback));
    auto db = drogon::app().getDbClient();

    auto rcb = [cb](const drogon::orm::Result&) {
        Json::Value resp;
        resp["success"] = true;
        (*cb)(drogon::HttpResponse::newHttpJsonResponse(resp));
    };
    auto ecb = [cb](const drogon::orm::DrogonDbException& e) {
        LOG_ERROR << "DB error in enrichProfile: " << e.base().what();
        Json::Value err;
        err["error"] = "Database error";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k500InternalServerError);
        (*cb)(resp);
    };

    // Dispatch based on param count (all values + userId).
    auto& p = params;
    switch (p.size()) {
        case 2:  db->execSqlAsync(sql, rcb, ecb, p[0], p[1]); break;
        case 3:  db->execSqlAsync(sql, rcb, ecb, p[0], p[1], p[2]); break;
        case 4:  db->execSqlAsync(sql, rcb, ecb, p[0], p[1], p[2], p[3]); break;
        case 5:  db->execSqlAsync(sql, rcb, ecb, p[0], p[1], p[2], p[3], p[4]); break;
        case 6:  db->execSqlAsync(sql, rcb, ecb, p[0], p[1], p[2], p[3], p[4], p[5]); break;
        case 7:  db->execSqlAsync(sql, rcb, ecb, p[0], p[1], p[2], p[3], p[4], p[5], p[6]); break;
        default: badRequest("Too many fields"); break;
    }
}
