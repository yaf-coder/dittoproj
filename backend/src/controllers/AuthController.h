#pragma once
#include <drogon/HttpController.h>

class AuthController : public drogon::HttpController<AuthController> {
public:
    METHOD_LIST_BEGIN
    // Fake login: POST {name, password} → returns {token}
    ADD_METHOD_TO(AuthController::login, "/api/auth/login", drogon::Post);
    // Returns the authenticated user's own profile
    ADD_METHOD_TO(AuthController::getMe, "/api/auth/me",    drogon::Get, "AuthFilter");
    METHOD_LIST_END

    void login(const drogon::HttpRequestPtr& req,
               std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void getMe(const drogon::HttpRequestPtr& req,
               std::function<void(const drogon::HttpResponsePtr&)>&& callback);
};
