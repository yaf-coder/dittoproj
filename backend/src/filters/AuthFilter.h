#pragma once
#include <drogon/HttpFilter.h>

// Validates the Bearer JWT in the Authorization header.
// On success, inserts "userId" (int64_t) into request attributes.
// On failure, responds with 401.
class AuthFilter : public drogon::HttpFilter<AuthFilter> {
public:
    void doFilter(const drogon::HttpRequestPtr& req,
                  drogon::FilterCallback&&      fcb,
                  drogon::FilterChainCallback&& fccb) override;
};
