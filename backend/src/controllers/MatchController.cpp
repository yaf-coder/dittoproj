#include "MatchController.h"
#include <drogon/drogon.h>
#include <json/json.h>
#include <trantor/utils/Logger.h>
#include <algorithm>

template<typename F>
auto shared_cb(F&& f) {
    return std::make_shared<std::decay_t<F>>(std::forward<F>(f));
}

static void dbError(const drogon::orm::DrogonDbException& e,
                    const char* context,
                    std::shared_ptr<std::function<void(const drogon::HttpResponsePtr&)>> cb) {
    LOG_ERROR << "DB error in " << context << ": " << e.base().what();
    Json::Value err;
    err["error"] = "Database error";
    auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
    resp->setStatusCode(drogon::k500InternalServerError);
    (*cb)(resp);
}

void MatchController::discover(const drogon::HttpRequestPtr& req,
                                std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    auto userId = req->attributes()->get<int64_t>("userId");
    auto cb     = shared_cb(std::move(callback));
    auto db     = drogon::app().getDbClient();

    // Return profiles the user hasn't liked yet, excluding incomplete profiles.
    db->execSqlAsync(
        "SELECT u.id, u.name, u.picture, u.age, u.gender"
        " FROM users u"
        " WHERE u.id != ?"
        "   AND u.id NOT IN (SELECT liked_id FROM likes WHERE liker_id = ?)"
        " ORDER BY RANDOM() LIMIT 10",
        [cb](const drogon::orm::Result& r) {
            Json::Value profiles(Json::arrayValue);
            for (const auto& row : r) {
                Json::Value p;
                p["id"]      = row["id"].as<int64_t>();
                p["name"]    = row["name"].as<std::string>();
                p["picture"] = row["picture"].isNull() ? "" : row["picture"].as<std::string>();
                if (!row["age"].isNull()) p["age"] = row["age"].as<int>();
                if (!row["gender"].isNull()) p["gender"] = row["gender"].as<std::string>();
                profiles.append(p);
            }
            (*cb)(drogon::HttpResponse::newHttpJsonResponse(profiles));
        },
        [cb](const drogon::orm::DrogonDbException& e) { dbError(e, "discover", cb); },
        userId, userId
    );
}

void MatchController::likeUser(const drogon::HttpRequestPtr& req,
                                std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                int64_t likedId) {
    auto userId = req->attributes()->get<int64_t>("userId");

    if (userId == likedId) {
        Json::Value err;
        err["error"] = "Cannot like yourself";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    auto cb = shared_cb(std::move(callback));
    auto db = drogon::app().getDbClient();

    db->execSqlAsync(
        "INSERT OR IGNORE INTO likes (liker_id, liked_id) VALUES (?, ?)",
        [cb, db, userId, likedId](const drogon::orm::Result&) {
            // Check whether the other person already liked us (mutual like = match).
            db->execSqlAsync(
                "SELECT id FROM likes WHERE liker_id = ? AND liked_id = ?",
                [cb, db, userId, likedId](const drogon::orm::Result& r) {
                    if (r.empty()) {
                        // No match yet — just confirm the like was recorded.
                        Json::Value resp;
                        resp["matched"] = false;
                        (*cb)(drogon::HttpResponse::newHttpJsonResponse(resp));
                        return;
                    }
                    // Mutual like — create a match row.
                    // Enforce user1_id < user2_id so the UNIQUE constraint works correctly.
                    int64_t u1 = std::min(userId, likedId);
                    int64_t u2 = std::max(userId, likedId);
                    db->execSqlAsync(
                        "INSERT OR IGNORE INTO matches (user1_id, user2_id) VALUES (?, ?)",
                        [cb](const drogon::orm::Result&) {
                            Json::Value resp;
                            resp["matched"] = true;
                            (*cb)(drogon::HttpResponse::newHttpJsonResponse(resp));
                        },
                        [cb](const drogon::orm::DrogonDbException& e) { dbError(e, "likeUser/createMatch", cb); },
                        u1, u2
                    );
                },
                [cb](const drogon::orm::DrogonDbException& e) { dbError(e, "likeUser/checkMutual", cb); },
                likedId, userId
            );
        },
        [cb](const drogon::orm::DrogonDbException& e) { dbError(e, "likeUser/insert", cb); },
        userId, likedId
    );
}

void MatchController::getCandidates(const drogon::HttpRequestPtr& req,
                                     std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    auto userId = req->attributes()->get<int64_t>("userId");
    auto cb     = shared_cb(std::move(callback));
    auto db     = drogon::app().getDbClient();

    db->execSqlAsync(
        "SELECT u.id, u.name, u.picture, u.age, u.gender,"
        "       COALESCE(cs.score, -1) AS compat_score"
        " FROM users u"
        " LEFT JOIN compatibility_scores cs ON cs.user_id = ? AND cs.match_id = u.id"
        " WHERE u.id != ?"
        "   AND u.id NOT IN (SELECT liked_id  FROM likes  WHERE liker_id  = ?)"
        "   AND u.id NOT IN (SELECT passed_id FROM passes WHERE passer_id = ?)"
        " ORDER BY compat_score DESC, RANDOM()"
        " LIMIT 20",
        [cb](const drogon::orm::Result& r) {
            Json::Value candidates(Json::arrayValue);
            for (const auto& row : r) {
                Json::Value c;
                c["id"]      = row["id"].as<int64_t>();
                c["name"]    = row["name"].as<std::string>();
                c["picture"] = row["picture"].isNull() ? "" : row["picture"].as<std::string>();
                if (!row["age"].isNull())    c["age"]    = row["age"].as<int>();
                if (!row["gender"].isNull()) c["gender"] = row["gender"].as<std::string>();
                double score = row["compat_score"].as<double>();
                if (score >= 0.0) c["compatibility"] = score;
                candidates.append(c);
            }
            (*cb)(drogon::HttpResponse::newHttpJsonResponse(candidates));
        },
        [cb](const drogon::orm::DrogonDbException& e) { dbError(e, "getCandidates", cb); },
        userId, userId, userId, userId
    );
}

void MatchController::passUser(const drogon::HttpRequestPtr& req,
                                std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                int64_t passedId) {
    auto userId = req->attributes()->get<int64_t>("userId");

    if (userId == passedId) {
        Json::Value err;
        err["error"] = "Cannot pass yourself";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    auto cb = shared_cb(std::move(callback));
    auto db = drogon::app().getDbClient();

    db->execSqlAsync(
        "INSERT OR IGNORE INTO passes (passer_id, passed_id) VALUES (?, ?)",
        [cb](const drogon::orm::Result&) {
            Json::Value resp;
            resp["success"] = true;
            (*cb)(drogon::HttpResponse::newHttpJsonResponse(resp));
        },
        [cb](const drogon::orm::DrogonDbException& e) { dbError(e, "passUser", cb); },
        userId, passedId
    );
}

void MatchController::getMatches(const drogon::HttpRequestPtr& req,
                                  std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    auto userId = req->attributes()->get<int64_t>("userId");
    auto cb     = shared_cb(std::move(callback));
    auto db     = drogon::app().getDbClient();

    db->execSqlAsync(
        "SELECT m.id AS match_id, m.created_at,"
        "       u.id AS user_id, u.name, u.picture, u.bio"
        " FROM matches m"
        " JOIN users u ON u.id = CASE WHEN m.user1_id = ? THEN m.user2_id ELSE m.user1_id END"
        " WHERE m.user1_id = ? OR m.user2_id = ?"
        " ORDER BY m.created_at DESC",
        [cb](const drogon::orm::Result& r) {
            Json::Value matches(Json::arrayValue);
            for (const auto& row : r) {
                Json::Value m;
                m["match_id"]    = row["match_id"].as<int64_t>();
                m["created_at"]  = row["created_at"].as<int64_t>();
                m["user"]["id"]      = row["user_id"].as<int64_t>();
                m["user"]["name"]    = row["name"].as<std::string>();
                m["user"]["picture"] = row["picture"].isNull() ? "" : row["picture"].as<std::string>();
                m["user"]["bio"]     = row["bio"].isNull()     ? "" : row["bio"].as<std::string>();
                matches.append(m);
            }
            (*cb)(drogon::HttpResponse::newHttpJsonResponse(matches));
        },
        [cb](const drogon::orm::DrogonDbException& e) { dbError(e, "getMatches", cb); },
        userId, userId, userId
    );
}
