#include <drogon/drogon.h>
#include <sqlite3.h>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "utils/JwtUtils.h"
#include "utils/OAuthUtils.h"
#include "utils/NamSorUtils.h"

// Run schema migrations against the SQLite file before Drogon opens its
// own connection pool. Uses the raw C API so we can enable WAL mode and
// run the SQL in one shot at startup.
static void runMigrations(const std::string& dbPath) {
    sqlite3* db = nullptr;
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::string err = sqlite3_errmsg(db);
        sqlite3_close(db);
        throw std::runtime_error("Cannot open DB for migration: " + err);
    }

    sqlite3_exec(db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "PRAGMA foreign_keys=ON;",  nullptr, nullptr, nullptr);

    std::ifstream file("migrations/001_initial.sql");
    if (!file.is_open()) {
        sqlite3_close(db);
        throw std::runtime_error("Cannot open migrations/001_initial.sql");
    }
    std::ostringstream ss;
    ss << file.rdbuf();

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, ss.str().c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::string err = errMsg ? errMsg : "unknown";
        sqlite3_free(errMsg);
        sqlite3_close(db);
        throw std::runtime_error("Migration failed: " + err);
    }

    sqlite3_close(db);
}

int main() {
    // loadConfigFile populates listeners, db_clients, and custom_config
    // without starting the event loop, so it's safe to read config here.
    drogon::app().loadConfigFile("config.json");

    const auto& cfg = drogon::app().getCustomConfig();

    JwtUtils::init(cfg["jwt_secret"].asString());
    OAuthUtils::init(
        cfg["google_client_id"].asString(),
        cfg["google_client_secret"].asString(),
        cfg["google_redirect_uri"].asString()
    );

    NamSorUtils::init(cfg["namsor_api_key"].asString());

    runMigrations(cfg["db_path"].asString());

    // Handle CORS preflight before routing so AuthFilter never sees OPTIONS.
    std::string frontendUrl = cfg["frontend_url"].asString();
    drogon::app().registerPreRoutingAdvice(
        [frontendUrl](const drogon::HttpRequestPtr& req,
                      drogon::AdviceCallback&&      acb,
                      drogon::AdviceChainCallback&& accb) {
            if (req->getMethod() == drogon::Options) {
                auto resp = drogon::HttpResponse::newHttpResponse();
                resp->setStatusCode(drogon::k204NoContent);
                resp->addHeader("Access-Control-Allow-Origin",  frontendUrl);
                resp->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
                resp->addHeader("Access-Control-Allow-Headers", "Authorization, Content-Type");
                acb(resp);
                return;
            }
            accb();
        }
    );

    // Add CORS headers to every non-OPTIONS response.
    drogon::app().registerPostHandlingAdvice(
        [frontendUrl](const drogon::HttpRequestPtr&,
                      const drogon::HttpResponsePtr& resp) {
            resp->addHeader("Access-Control-Allow-Origin",  frontendUrl);
            resp->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
            resp->addHeader("Access-Control-Allow-Headers", "Authorization, Content-Type");
        }
    );

    LOG_INFO << "Starting Ditto backend...";
    drogon::app().run();
    return 0;
}
