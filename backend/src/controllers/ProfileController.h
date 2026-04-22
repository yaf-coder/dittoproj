#pragma once
#include <drogon/HttpController.h>

class ProfileController : public drogon::HttpController<ProfileController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(ProfileController::getProfile,    "/api/profiles/{id}",    drogon::Get,  "AuthFilter");
    ADD_METHOD_TO(ProfileController::updateProfile, "/api/profiles/me",      drogon::Put,  "AuthFilter");
    ADD_METHOD_TO(ProfileController::enrichProfile, "/api/profiles/me/enrich", drogon::Put, "AuthFilter");
    METHOD_LIST_END

    void getProfile(const drogon::HttpRequestPtr& req,
                    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                    int64_t id);

    void updateProfile(const drogon::HttpRequestPtr& req,
                       std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    // Optional "Enrich Your Profile" form — all fields are optional.
    void enrichProfile(const drogon::HttpRequestPtr& req,
                       std::function<void(const drogon::HttpResponsePtr&)>&& callback);
};
