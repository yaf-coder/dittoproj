#include "AuthFilter.h"
#include "utils/JwtUtils.h"
#include <drogon/HttpResponse.h>
#include <json/json.h>

void AuthFilter::doFilter(const drogon::HttpRequestPtr& req,
                           drogon::FilterCallback&&      fcb,
                           drogon::FilterChainCallback&& fccb) {
    auto authHeader = req->getHeader("Authorization");

    auto reject = [&fcb](drogon::HttpStatusCode code, const char* msg) {
        Json::Value body;
        body["error"] = msg;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(body);
        resp->setStatusCode(code);
        fcb(resp);
    };

    if (authHeader.size() < 8 || authHeader.substr(0, 7) != "Bearer ") {
        reject(drogon::k401Unauthorized, "Unauthorized");
        return;
    }

    auto userId = JwtUtils::validateToken(authHeader.substr(7));
    if (!userId) {
        reject(drogon::k401Unauthorized, "Invalid or expired token");
        return;
    }

    req->attributes()->insert("userId", *userId);
    fccb();
}
