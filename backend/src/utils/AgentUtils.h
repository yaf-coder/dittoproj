#pragma once
#include <drogon/HttpClient.h>
#include <drogon/HttpRequest.h>
#include <json/json.h>
#include <trantor/utils/Logger.h>

// Fire-and-forget: notifies the Python compatibility agent to recompute
// scores for the given user. Errors are logged but never surface to the caller.
class AgentUtils {
public:
    static void triggerCompatibility(int64_t userId) {
        auto client = drogon::HttpClient::newHttpClient("http://localhost:8081");
        Json::Value body;
        body["user_id"] = userId;
        auto req = drogon::HttpRequest::newHttpJsonRequest(body);
        req->setMethod(drogon::Post);
        req->setPath("/compute");
        client->sendRequest(req,
            [userId](drogon::ReqResult result, const drogon::HttpResponsePtr& resp) {
                if (result != drogon::ReqResult::Ok)
                    LOG_WARN << "Agent trigger failed for user " << userId;
            }
        );
    }
};
