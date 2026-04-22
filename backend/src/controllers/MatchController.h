#pragma once
#include <drogon/HttpController.h>

class MatchController : public drogon::HttpController<MatchController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(MatchController::discover,    "/api/discover",    drogon::Get,  "AuthFilter");
    ADD_METHOD_TO(MatchController::getCandidates, "/api/candidates", drogon::Get, "AuthFilter");
    ADD_METHOD_TO(MatchController::likeUser,    "/api/likes/{id}",  drogon::Post, "AuthFilter");
    ADD_METHOD_TO(MatchController::passUser,    "/api/passes/{id}", drogon::Post, "AuthFilter");
    ADD_METHOD_TO(MatchController::getMatches,  "/api/matches",     drogon::Get,  "AuthFilter");
    METHOD_LIST_END

    void discover(const drogon::HttpRequestPtr& req,
                  std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    // Returns candidates ordered by compatibility score descending.
    void getCandidates(const drogon::HttpRequestPtr& req,
                       std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void likeUser(const drogon::HttpRequestPtr& req,
                  std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                  int64_t id);

    // Record a pass (skip) without liking.
    void passUser(const drogon::HttpRequestPtr& req,
                  std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                  int64_t id);

    void getMatches(const drogon::HttpRequestPtr& req,
                    std::function<void(const drogon::HttpResponsePtr&)>&& callback);
};
