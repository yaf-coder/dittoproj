#include "EnrichmentService.h"
#include "utils/NamSorUtils.h"
#include "utils/EducationUtils.h"
#include "utils/PoliticalBeliefUtils.h"
#include <trantor/utils/Logger.h>
#include <atomic>
#include <memory>

void EnrichmentService::enrich(int64_t                  userId,
                                const std::string&       fullName,
                                int                      age,
                                const std::string&       location,
                                drogon::orm::DbClientPtr db) {
    // NamSor gender and ethnicity are independent — fire them in parallel.
    // A shared atomic counter lets us detect when both have finished.
    struct NamSorState {
        std::string        gender    = "unknown";
        std::string        ethnicity = "unknown";
        std::atomic<int>   pending{2};
    };
    auto state = std::make_shared<NamSorState>();

    // Called once per NamSor response. Proceeds to education + political
    // belief computation only after both calls have returned.
    auto onNamSorDone = [state, userId, age, fullName, location, db]() {
        // fetch_sub returns the value *before* the decrement, so the last
        // caller sees 1 → proceeds; the first caller sees 2 → returns early.
        if (state->pending.fetch_sub(1) != 1) return;

        // --- Education lookup (waterfall: Apollo → LinkedIn → web) ----------
        EducationUtils::lookup(fullName, location,
            [state, userId, age, location, db](std::string educationLevel) {

                // --- Political belief (synchronous formula) ------------------
                float politicalBelief = PoliticalBeliefUtils::compute({
                    state->gender,
                    state->ethnicity,
                    educationLevel,
                    location,
                    age
                });

                // --- Persist enriched attributes ----------------------------
                db->execSqlAsync(
                    "UPDATE users"
                    " SET projected_gender    = ?,"
                    "     projected_ethnicity = ?,"
                    "     education_level     = ?,"
                    "     political_belief    = ?,"
                    "     enrichment_status   = 'done',"
                    "     enriched_at         = unixepoch()"
                    " WHERE id = ?",
                    [userId](const drogon::orm::Result&) {
                        LOG_INFO << "Enrichment complete for user " << userId;
                    },
                    [userId](const drogon::orm::DrogonDbException& e) {
                        LOG_ERROR << "DB error saving enrichment for user "
                                  << userId << ": " << e.base().what();
                    },
                    state->gender,
                    state->ethnicity,
                    educationLevel,
                    politicalBelief,
                    userId
                );
            }
        );
    };

    // Fire both NamSor calls — whichever finishes last triggers onNamSorDone.
    NamSorUtils::inferGender(fullName, [state, onNamSorDone](std::optional<std::string> g) {
        if (g) state->gender = *g;
        onNamSorDone();
    });

    NamSorUtils::inferEthnicity(fullName, [state, onNamSorDone](std::optional<std::string> e) {
        if (e) state->ethnicity = *e;
        onNamSorDone();
    });
}
