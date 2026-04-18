#include "NamSorUtils.h"
#include <drogon/HttpClient.h>
#include <drogon/HttpRequest.h>
#include <json/json.h>
#include <trantor/utils/Logger.h>

void NamSorUtils::init(const std::string& apiKey) {
    apiKey_ = apiKey;
}

std::pair<std::string, std::string> NamSorUtils::splitName(const std::string& fullName) {
    auto pos = fullName.find(' ');
    if (pos == std::string::npos) {
        return {fullName, ""};
    }
    return {fullName.substr(0, pos), fullName.substr(pos + 1)};
}

// Builds a NamSor request body with a single name entry.
static std::string buildNameBody(const std::string& firstName, const std::string& lastName) {
    Json::Value root;
    Json::Value entry;
    entry["id"]        = "0";
    entry["firstName"] = firstName;
    entry["lastName"]  = lastName;
    root["personalNames"].append(entry);

    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    return Json::writeString(builder, root);
}

void NamSorUtils::inferGender(const std::string& fullName,
                               std::function<void(std::optional<std::string>)> callback) {
    auto [firstName, lastName] = splitName(fullName);

    auto client = drogon::HttpClient::newHttpClient("https://v2.namsor.com");
    auto req    = drogon::HttpRequest::newHttpRequest();
    req->setMethod(drogon::Post);
    req->setPath("/NamSorAPIv2/api2/json/genderBatch");
    req->addHeader("Content-Type", "application/json");
    req->addHeader("Accept",       "application/json");
    req->addHeader("X-API-KEY",    apiKey_);
    req->setBody(buildNameBody(firstName, lastName));

    client->sendRequest(req,
        [cb = std::move(callback)](drogon::ReqResult result,
                                   const drogon::HttpResponsePtr& resp) mutable {
            if (result != drogon::ReqResult::Ok || !resp ||
                resp->statusCode() != drogon::k200OK) {
                LOG_WARN << "NamSor genderBatch failed, result=" << static_cast<int>(result);
                cb(std::nullopt);
                return;
            }

            Json::Value json;
            Json::Reader reader;
            if (!reader.parse(resp->body(), json) ||
                !json.isMember("personalNames") ||
                json["personalNames"].empty()) {
                LOG_WARN << "NamSor genderBatch: unexpected response body";
                cb(std::nullopt);
                return;
            }

            auto gender = json["personalNames"][0]["likelyGender"].asString();
            cb(gender.empty() ? std::nullopt : std::optional<std::string>{gender});
        }
    );
}

void NamSorUtils::inferEthnicity(const std::string& fullName,
                                  std::function<void(std::optional<std::string>)> callback) {
    auto [firstName, lastName] = splitName(fullName);

    auto client = drogon::HttpClient::newHttpClient("https://v2.namsor.com");
    auto req    = drogon::HttpRequest::newHttpRequest();
    req->setMethod(drogon::Post);
    req->setPath("/NamSorAPIv2/api2/json/usRaceEthnicityBatch");
    req->addHeader("Content-Type", "application/json");
    req->addHeader("Accept",       "application/json");
    req->addHeader("X-API-KEY",    apiKey_);
    req->setBody(buildNameBody(firstName, lastName));

    client->sendRequest(req,
        [cb = std::move(callback)](drogon::ReqResult result,
                                   const drogon::HttpResponsePtr& resp) mutable {
            if (result != drogon::ReqResult::Ok || !resp ||
                resp->statusCode() != drogon::k200OK) {
                LOG_WARN << "NamSor usRaceEthnicityBatch failed, result=" << static_cast<int>(result);
                cb(std::nullopt);
                return;
            }

            Json::Value json;
            Json::Reader reader;
            if (!reader.parse(resp->body(), json) ||
                !json.isMember("personalNames") ||
                json["personalNames"].empty()) {
                LOG_WARN << "NamSor usRaceEthnicityBatch: unexpected response body";
                cb(std::nullopt);
                return;
            }

            auto ethnicity = json["personalNames"][0]["raceEthnicity"].asString();
            cb(ethnicity.empty() ? std::nullopt : std::optional<std::string>{ethnicity});
        }
    );
}
