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
        "male", "female", "non_binary", "prefer_not_to_say"
    };
    static const std::set<std::string> validEthnicities = {
        "white", "black", "hispanic", "asian",
        "native_american", "pacific_islander", "middle_eastern",
        "other", "prefer_not_to_say"
    };
    static const std::set<std::string> validEducation = {
        "high_school", "some_college", "bachelors", "masters", "phd"
    };

    // All fields are optional — only update what was provided.
    std::string setClauses;
    std::vector<std::string> stringParams;
    std::optional<float> politicalBelief;

    if (body->isMember("gender")) {
        std::string v = (*body)["gender"].asString();
        if (validGenders.find(v) == validGenders.end()) {
            badRequest("gender must be one of: male, female, non_binary, prefer_not_to_say");
            return;
        }
        setClauses += (setClauses.empty() ? "" : ", ") + std::string("gender=?");
        stringParams.push_back(v);
    }
    if (body->isMember("ethnicity")) {
        std::string v = (*body)["ethnicity"].asString();
        if (validEthnicities.find(v) == validEthnicities.end()) {
            badRequest("ethnicity must be one of: white, black, hispanic, asian, native_american, pacific_islander, middle_eastern, other, prefer_not_to_say");
            return;
        }
        setClauses += (setClauses.empty() ? "" : ", ") + std::string("ethnicity=?");
        stringParams.push_back(v);
    }
    if (body->isMember("education_level")) {
        std::string v = (*body)["education_level"].asString();
        if (validEducation.find(v) == validEducation.end()) {
            badRequest("education_level must be one of: high_school, some_college, bachelors, masters, phd");
            return;
        }
        setClauses += (setClauses.empty() ? "" : ", ") + std::string("education_level=?");
        stringParams.push_back(v);
    }
    if (body->isMember("political_belief")) {
        float v = (*body)["political_belief"].asFloat();
        if (v < 0.0f || v > 1.0f) {
            badRequest("political_belief must be between 0.0 and 1.0");
            return;
        }
        politicalBelief = v;
        setClauses += (setClauses.empty() ? "" : ", ") + std::string("political_belief=?");
    }

    if (setClauses.empty()) {
        badRequest("No fields provided; send at least one of: gender, ethnicity, education_level, political_belief");
        return;
    }

    setClauses += ", updated_at=unixepoch()";
    std::string sql = "UPDATE users SET " + setClauses + " WHERE id=?";

    auto cb = shared_cb(std::move(callback));
    auto db = drogon::app().getDbClient();

    // Build the argument list dynamically. String params come first (in order),
    // then political_belief (if present), then userId.
    auto exec = [&](auto&&... args) {
        db->execSqlAsync(
            sql,
            [cb](const drogon::orm::Result&) {
                Json::Value resp;
                resp["success"] = true;
                (*cb)(drogon::HttpResponse::newHttpJsonResponse(resp));
            },
            [cb](const drogon::orm::DrogonDbException& e) {
                LOG_ERROR << "DB error in enrichProfile: " << e.base().what();
                Json::Value err;
                err["error"] = "Database error";
                auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
                resp->setStatusCode(drogon::k500InternalServerError);
                (*cb)(resp);
            },
            std::forward<decltype(args)>(args)...
        );
    };

    // Drogon's execSqlAsync takes variadic typed args, not a runtime vector.
    // We build the call with up to 4 string params + optional float + userId.
    // Enumerate the cases by param count.
    int strCount = static_cast<int>(stringParams.size());
    bool hasPB = politicalBelief.has_value();
    float pb = hasPB ? *politicalBelief : 0.0f;

    // Helper lambdas to reduce repetition.
    auto s = [&](int i) -> const std::string& { return stringParams[i]; };

    if      (strCount == 0 && !hasPB) { /* unreachable; caught above */ }
    else if (strCount == 0 &&  hasPB) exec(pb, userId);
    else if (strCount == 1 && !hasPB) exec(s(0), userId);
    else if (strCount == 1 &&  hasPB) exec(s(0), pb, userId);
    else if (strCount == 2 && !hasPB) exec(s(0), s(1), userId);
    else if (strCount == 2 &&  hasPB) exec(s(0), s(1), pb, userId);
    else if (strCount == 3 && !hasPB) exec(s(0), s(1), s(2), userId);
    else                              exec(s(0), s(1), s(2), pb, userId); // strCount==3, hasPB
}
