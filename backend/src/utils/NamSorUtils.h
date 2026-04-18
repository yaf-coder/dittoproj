#pragma once
#include <functional>
#include <optional>
#include <string>

// Thin async wrapper around the NamSor v2 API.
// Two independent calls are made per user (gender + ethnicity) so they
// should be fired in parallel — see EnrichmentService for the pattern.
class NamSorUtils {
public:
    static void init(const std::string& apiKey);

    // Infers binary gender ("male" | "female") from a full name.
    // Calls back with nullopt on API or parse failure.
    static void inferGender(
        const std::string& fullName,
        std::function<void(std::optional<std::string>)> callback
    );

    // Infers US race/ethnicity code from a full name.
    // Codes: "W_NL" (White non-Latino), "HL" (Hispanic/Latino),
    //        "A" (Asian), "B_NL" (Black non-Latino), "AI_AN", "PI".
    // Calls back with nullopt on API or parse failure.
    static void inferEthnicity(
        const std::string& fullName,
        std::function<void(std::optional<std::string>)> callback
    );

private:
    static inline std::string apiKey_;

    // Splits "John Smith Jr" into {"John", "Smith Jr"}.
    static std::pair<std::string, std::string> splitName(const std::string& fullName);
};
