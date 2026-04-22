#include <drogon/drogon.h>
#include <sqlite3.h>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <string>
#include "utils/JwtUtils.h"

static void runMigration(sqlite3* db, const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error("Cannot open migration file: " + path);
    std::ostringstream ss;
    ss << file.rdbuf();

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, ss.str().c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::string err = errMsg ? errMsg : "unknown";
        sqlite3_free(errMsg);
        throw std::runtime_error("Migration failed (" + path + "): " + err);
    }
}

static void runMigrations(const std::string& dbPath) {
    // Must configure threading before any sqlite3_open call, otherwise
    // sqlite3_initialize() fires implicitly and locks in the default config,
    // causing Drogon's later sqlite3_config(SQLITE_CONFIG_MULTITHREAD) to fail.
    sqlite3_config(SQLITE_CONFIG_MULTITHREAD);

    sqlite3* db = nullptr;
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::string err = sqlite3_errmsg(db);
        sqlite3_close(db);
        throw std::runtime_error("Cannot open DB for migration: " + err);
    }

    sqlite3_exec(db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "PRAGMA foreign_keys=ON;",  nullptr, nullptr, nullptr);

    const std::vector<std::string> migrations = {
        "migrations/001_initial.sql",
        "migrations/005_user_responses.sql",
        "migrations/006_compatibility_scores.sql",
        "migrations/007_passes.sql",
    };

    for (const auto& path : migrations) {
        runMigration(db, path);
    }

    sqlite3_close(db);
}

int main() {
    drogon::app().loadConfigFile("config.json");

    const auto& cfg = drogon::app().getCustomConfig();

    JwtUtils::init(cfg["jwt_secret"].asString());

    runMigrations(cfg["db_path"].asString());

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
