#pragma once
#include <drogon/HttpController.h>

class AuthController : public drogon::HttpController<AuthController> {
public:
    METHOD_LIST_BEGIN
    // Phone + password login — POST { phone_number, password } → { token }
    ADD_METHOD_TO(AuthController::login, "/api/auth/login", drogon::Post);
    // Returns the authenticated user's own profile
    ADD_METHOD_TO(AuthController::getMe, "/api/auth/me",    drogon::Get, "AuthFilter");

    // -------------------------------------------------------------------------
    // Google OAuth (commented out — re-enable by uncommenting and adding
    // OAuthUtils::init() back to main.cpp)
    // -------------------------------------------------------------------------
    // ADD_METHOD_TO(AuthController::googleLogin,    "/auth/google",          drogon::Get);
    // ADD_METHOD_TO(AuthController::googleCallback, "/auth/google/callback", drogon::Get);
    METHOD_LIST_END

    void login(const drogon::HttpRequestPtr& req,
               std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void getMe(const drogon::HttpRequestPtr& req,
               std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    // -------------------------------------------------------------------------
    // Google OAuth handlers (commented out)
    // -------------------------------------------------------------------------
    // void googleLogin(const drogon::HttpRequestPtr& req,
    //                  std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    // void googleCallback(const drogon::HttpRequestPtr& req,
    //                     std::function<void(const drogon::HttpResponsePtr&)>&& callback);
};
