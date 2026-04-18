#pragma once
#include <drogon/HttpController.h>

class MatchController : public drogon::HttpController<MatchController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(MatchController::discover,   "/api/discover",    drogon::Get,  "AuthFilter");
    ADD_METHOD_TO(MatchController::likeUser,   "/api/likes/{id}",  drogon::Post, "AuthFilter");
    ADD_METHOD_TO(MatchController::getMatches, "/api/matches",     drogon::Get,  "AuthFilter");
    METHOD_LIST_END

    // Returns up to 10 candidate profiles the user hasn't swiped on yet.
    void discover(const drogon::HttpRequestPtr& req,
                  std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    // Like a user. Responds with { matched: bool }.
    void likeUser(const drogon::HttpRequestPtr& req,
                  std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                  int64_t id);

    // Returns all mutual matches with basic user info.
    void getMatches(const drogon::HttpRequestPtr& req,
                    std::function<void(const drogon::HttpResponsePtr&)>&& callback);
};
