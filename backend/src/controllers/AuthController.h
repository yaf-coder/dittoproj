#pragma once
#include <drogon/HttpController.h>

class AuthController : public drogon::HttpController<AuthController> {
public:
    METHOD_LIST_BEGIN
    // OAuth entry point — redirects the browser to Google
    ADD_METHOD_TO(AuthController::googleLogin,    "/auth/google",          drogon::Get);
    // Google redirects back here with ?code=...
    ADD_METHOD_TO(AuthController::googleCallback, "/auth/google/callback", drogon::Get);
    // Returns the authenticated user's own profile
    ADD_METHOD_TO(AuthController::getMe,          "/api/auth/me",          drogon::Get, "AuthFilter");
    METHOD_LIST_END

    void googleLogin(const drogon::HttpRequestPtr& req,
                     std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void googleCallback(const drogon::HttpRequestPtr& req,
                        std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void getMe(const drogon::HttpRequestPtr& req,
               std::function<void(const drogon::HttpResponsePtr&)>&& callback);
};
