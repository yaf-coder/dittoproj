#pragma once
#include <functional>
#include <string>

// Attempts to infer a person's highest education level from their name and
// location by querying third-party data sources. All lookups are async.
//
// Education level values (ordered):
//   "high_school" | "some_college" | "bachelors" | "masters" | "phd" | "unknown"
class EducationUtils {
public:
    // Runs the lookup pipeline: Apollo.io → LinkedIn scrape → web scrape.
    // Calls back with "unknown" if all sources fail.
    static void lookup(
        const std::string& fullName,
        const std::string& location,
        std::function<void(std::string)> callback
    );

private:
    // PSEUDOCODE stubs — replace with real implementations.
    static void queryApollo(
        const std::string& fullName,
        const std::string& location,
        std::function<void(std::string)> callback
    );

    static void scrapeLinkedIn(
        const std::string& fullName,
        const std::string& location,
        std::function<void(std::string)> callback
    );

    static void scrapeWeb(
        const std::string& fullName,
        const std::string& location,
        std::function<void(std::string)> callback
    );
};
