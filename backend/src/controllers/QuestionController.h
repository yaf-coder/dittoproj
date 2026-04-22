#pragma once
#include <drogon/HttpController.h>

class QuestionController : public drogon::HttpController<QuestionController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(QuestionController::getRandom, "/api/questions/random", drogon::Get,  "AuthFilter");
    ADD_METHOD_TO(QuestionController::answer,    "/api/questions/answer", drogon::Post, "AuthFilter");
    METHOD_LIST_END

    // Returns a random question, preferring ones the user hasn't answered yet.
    void getRandom(const drogon::HttpRequestPtr& req,
                   std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    // Body: { "question_id": int, "answer": string }
    void answer(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback);
};
