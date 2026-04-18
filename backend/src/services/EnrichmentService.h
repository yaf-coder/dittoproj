#pragma once
#include <drogon/orm/DbClient.h>
#include <string>

// Runs the full enrichment pipeline for a user asynchronously.
// Fire-and-forget: the caller gets a response immediately; enrichment
// writes results back to the DB when all lookups complete.
class EnrichmentService {
public:
    static void enrich(
        int64_t                        userId,
        const std::string&             fullName,
        int                            age,
        const std::string&             location,
        drogon::orm::DbClientPtr       db
    );
};
