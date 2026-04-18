#include "EducationUtils.h"
#include <trantor/utils/Logger.h>

// ---------------------------------------------------------------------------
// Public entry point — waterfall: Apollo → LinkedIn → web → unknown
// ---------------------------------------------------------------------------
void EducationUtils::lookup(const std::string& fullName,
                             const std::string& location,
                             std::function<void(std::string)> callback) {
    queryApollo(fullName, location,
        [fullName, location, callback](std::string edu) {
            if (edu != "unknown") { callback(edu); return; }

            scrapeLinkedIn(fullName, location,
                [fullName, location, callback](std::string edu) {
                    if (edu != "unknown") { callback(edu); return; }

                    scrapeWeb(fullName, location, callback);
                }
            );
        }
    );
}

// ---------------------------------------------------------------------------
// PSEUDOCODE: Apollo.io person enrichment API
//
// Real implementation outline:
//   POST https://api.apollo.io/v1/people/match
//   Headers: { "Content-Type": "application/json", "X-Api-Key": <key> }
//   Body: { "name": fullName, "location": location, "reveal_personal_emails": false }
//
//   Response path: person.employment_history[*].education_level
//   OR:            person.education[*].degree  (map to our enum)
//
// Degree mapping:
//   "High School Diploma" / "GED"          → "high_school"
//   "Associate"                             → "some_college"
//   "Bachelor" / "B.S." / "B.A."           → "bachelors"
//   "Master" / "M.S." / "MBA" / "M.A."    → "masters"
//   "Doctor" / "Ph.D" / "J.D." / "M.D."  → "phd"
// ---------------------------------------------------------------------------
void EducationUtils::queryApollo(const std::string& fullName,
                                  const std::string& location,
                                  std::function<void(std::string)> callback) {
    // FAKE API CALL — replace with real Drogon async HTTP request
    // auto client = drogon::HttpClient::newHttpClient("https://api.apollo.io");
    // auto req    = drogon::HttpRequest::newHttpRequest();
    // req->setMethod(drogon::Post);
    // req->setPath("/v1/people/match");
    // req->addHeader("X-Api-Key", apolloApiKey_);
    // req->addHeader("Content-Type", "application/json");
    // req->setBody(R"({"name":")" + fullName + R"(","location":")" + location + R"("})");
    // client->sendRequest(req, [callback](drogon::ReqResult result, ...) {
    //     auto degree = response["person"]["education"][0]["degree"].asString();
    //     callback(mapDegreeToLevel(degree));
    // });

    LOG_DEBUG << "[EducationUtils] Apollo lookup not implemented — returning unknown";
    callback("unknown");
}

// ---------------------------------------------------------------------------
// PSEUDOCODE: LinkedIn profile scraper
//
// Real implementation outline:
//   1. Search Google for: site:linkedin.com/in "<fullName>" "<location>"
//   2. Fetch the first matching LinkedIn URL via a headless browser or
//      a LinkedIn scraping service (e.g. Proxycurl, ScrapingBee).
//   3. Parse the "Education" section from the profile HTML.
//   4. Map degree strings → our enum using the same mapping as Apollo above.
//
// NOTE: LinkedIn's ToS prohibits scraping. Use the official LinkedIn API
// (requires a partner agreement) or a compliant third-party service.
// ---------------------------------------------------------------------------
void EducationUtils::scrapeLinkedIn(const std::string& fullName,
                                     const std::string& location,
                                     std::function<void(std::string)> callback) {
    // FAKE SCRAPE — replace with real implementation
    // std::string searchQuery = "site:linkedin.com/in " + fullName + " " + location;
    // webSearch(searchQuery, [=](std::string linkedInUrl) {
    //     fetchPage(linkedInUrl, [=](std::string html) {
    //         auto degree = parseLinkedInEducation(html);
    //         callback(mapDegreeToLevel(degree));
    //     });
    // });

    LOG_DEBUG << "[EducationUtils] LinkedIn scrape not implemented — returning unknown";
    callback("unknown");
}

// ---------------------------------------------------------------------------
// PSEUDOCODE: General web scraper fallback
//
// Real implementation outline:
//   1. Query a search engine API (SerpAPI, Bing Search API, etc.) for:
//      "<fullName>" "<location>" (education OR university OR degree OR college)
//   2. Parse the top N result snippets for degree keywords.
//   3. Return the highest detected degree, or "unknown".
// ---------------------------------------------------------------------------
void EducationUtils::scrapeWeb(const std::string& fullName,
                                const std::string& location,
                                std::function<void(std::string)> callback) {
    // FAKE WEB SEARCH — replace with real implementation
    // std::string query = "\"" + fullName + "\" \"" + location + "\" education degree university";
    // serpApi->search(query, [=](SearchResults results) {
    //     for (auto& snippet : results.snippets) {
    //         auto level = extractEducationKeyword(snippet);
    //         if (level != "unknown") { callback(level); return; }
    //     }
    //     callback("unknown");
    // });

    LOG_DEBUG << "[EducationUtils] Web scrape not implemented — returning unknown";
    callback("unknown");
}
