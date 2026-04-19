#include "QuestionController.h"
#include <drogon/drogon.h>
#include <json/json.h>
#include <trantor/utils/Logger.h>
#include <vector>
#include <string>
#include <algorithm>
#include <random>

template<typename F>
auto shared_cb(F&& f) {
    return std::make_shared<std::decay_t<F>>(std::forward<F>(f));
}

struct Question {
    int         id;
    std::string text;
};

static const std::vector<Question> QUESTIONS = {
    { 1,  "What does your ideal free weekend usually look like?" },
    { 2,  "When you're stressed or overwhelmed, what do you usually do?" },
    { 3,  "What does a really good relationship look like to you day-to-day?" },
    { 4,  "Tell me about a time you disagreed with someone close to you — how'd that go?" },
    { 5,  "What are you most focused on in your life right now?" },
    { 6,  "How important is ambition or career stuff in a partner to you?" },
    { 7,  "What are some things you just won't compromise on in a relationship?" },
    { 8,  "How do you usually show and like to receive affection?" },
    { 9,  "What's your ideal social life like?" },
    { 10, "What's something about you people don't realize right away?" },
};

void QuestionController::getRandom(const drogon::HttpRequestPtr& req,
                                    std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    auto userId = req->attributes()->get<int64_t>("userId");
    auto cb     = shared_cb(std::move(callback));
    auto db     = drogon::app().getDbClient();

    // Fetch which question IDs this user has already answered.
    db->execSqlAsync(
        "SELECT DISTINCT question_id FROM user_responses WHERE user_id = ?",
        [cb, userId](const drogon::orm::Result& r) {
            std::vector<int> answered;
            for (const auto& row : r)
                answered.push_back(row["question_id"].as<int>());

            // Build pool of unanswered questions; fall back to full list if all answered.
            std::vector<const Question*> pool;
            for (const auto& q : QUESTIONS) {
                if (std::find(answered.begin(), answered.end(), q.id) == answered.end())
                    pool.push_back(&q);
            }
            if (pool.empty()) {
                for (const auto& q : QUESTIONS)
                    pool.push_back(&q);
            }

            // Pick a random question from the pool.
            std::random_device rd;
            std::mt19937 rng(rd());
            std::uniform_int_distribution<size_t> dist(0, pool.size() - 1);
            const Question* q = pool[dist(rng)];

            Json::Value resp;
            resp["id"]   = q->id;
            resp["text"] = q->text;
            (*cb)(drogon::HttpResponse::newHttpJsonResponse(resp));
        },
        [cb](const drogon::orm::DrogonDbException& e) {
            LOG_ERROR << "DB error in getRandom: " << e.base().what();
            Json::Value err;
            err["error"] = "Database error";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(drogon::k500InternalServerError);
            (*cb)(resp);
        },
        userId
    );
}

void QuestionController::answer(const drogon::HttpRequestPtr& req,
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

    int         questionId = (*body).get("question_id", -1).asInt();
    std::string answer     = (*body).get("answer", "").asString();

    if (questionId < 1 || questionId > (int)QUESTIONS.size()) {
        badRequest("Invalid question_id");
        return;
    }
    if (answer.empty()) {
        badRequest("answer is required");
        return;
    }

    auto cb = shared_cb(std::move(callback));
    auto db = drogon::app().getDbClient();

    db->execSqlAsync(
        "INSERT INTO user_responses (user_id, question_id, answer) VALUES (?, ?, ?)",
        [cb](const drogon::orm::Result&) {
            Json::Value resp;
            resp["success"] = true;
            (*cb)(drogon::HttpResponse::newHttpJsonResponse(resp));
        },
        [cb](const drogon::orm::DrogonDbException& e) {
            LOG_ERROR << "DB error in answer: " << e.base().what();
            Json::Value err;
            err["error"] = "Database error";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(drogon::k500InternalServerError);
            (*cb)(resp);
        },
        userId, questionId, answer
    );
}
