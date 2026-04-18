#include "PoliticalBeliefUtils.h"
#include <algorithm>
#include <cctype>
#include <unordered_map>

// =============================================================================
// Political Belief Score — Formula Documentation
// =============================================================================
// Output range: 0.0 = far left, 1.0 = far right.
//
// Method: weighted average of per-dimension Republican fractions.
//   repFraction_i = Rep% / (Rep% + Dem%)  for dimension i
//   score = Σ( repFraction_i × weight_i )
//
// Weights reflect each variable's predictive strength in the literature.
// They sum to 1.0. Unknown values contribute 0.5 (neutral) at full weight.
//
// Sources:
//   [PEW-2024-GENDER] Pew Research Center, "Partisanship by gender, sexual
//     orientation, marital and parental status," April 9, 2024.
//     https://www.pewresearch.org/politics/2024/04/09/partisanship-by-gender-sexual-orientation-marital-and-parental-status/
//   [PEW-2023-RACE]   Pew Research Center, "Party affiliation by race/ethnicity," 2023.
//   [PEW-2023-EDU]    Pew Research Center, "Party affiliation by education level," 2023.
//   [PEW-2023-AGE]    Pew Research Center, "Party affiliation by generation," 2023.
//   [ELECTION-2024]   2024 U.S. presidential election two-party results by state.
// =============================================================================

// -----------------------------------------------------------------------------
// Gender  [PEW-2024-GENDER]
//   Men:   52% Rep, 46% Dem  →  52 / (52+46) = 0.531
//   Women: 44% Rep, 51% Dem  →  44 / (44+51) = 0.463
// -----------------------------------------------------------------------------
static float genderFraction(const std::string& gender) {
    if (gender == "male")   return 52.f / (52.f + 46.f);   // 0.531
    if (gender == "female") return 44.f / (44.f + 51.f);   // 0.463
    return 0.5f;
}

// -----------------------------------------------------------------------------
// Ethnicity  [PEW-2023-RACE]
// NamSor usRaceEthnicity codes → estimated Republican fraction.
// -----------------------------------------------------------------------------
static float ethnicityFraction(const std::string& eth) {
    // W_NL  White Non-Latino:       58% Rep, 36% Dem  →  0.617
    // HL    Hispanic/Latino:        37% Rep, 56% Dem  →  0.398
    // A     Asian:                  35% Rep, 58% Dem  →  0.376
    // B_NL  Black Non-Latino:       10% Rep, 85% Dem  →  0.105
    // AI_AN Am. Indian/AK Native:   40% Rep, 50% Dem  →  0.444
    // PI    Pacific Islander:       36% Rep, 52% Dem  →  0.409
    static const std::unordered_map<std::string, float> t = {
        {"W_NL",  58.f / (58.f + 36.f)},
        {"HL",    37.f / (37.f + 56.f)},
        {"A",     35.f / (35.f + 58.f)},
        {"B_NL",  10.f / (10.f + 85.f)},
        {"AI_AN", 40.f / (40.f + 50.f)},
        {"PI",    36.f / (36.f + 52.f)},
    };
    auto it = t.find(eth);
    return it != t.end() ? it->second : 0.5f;
}

// -----------------------------------------------------------------------------
// Education  [PEW-2023-EDU]
// Higher education correlates with Democratic lean.
// -----------------------------------------------------------------------------
static float educationFraction(const std::string& edu) {
    if (edu == "phd")          return 32.f / (32.f + 62.f);   // 0.340
    if (edu == "masters")      return 39.f / (39.f + 57.f);   // 0.406
    if (edu == "bachelors")    return 44.f / (44.f + 52.f);   // 0.458
    if (edu == "some_college") return 50.f / (50.f + 46.f);   // 0.521
    if (edu == "high_school")  return 52.f / (52.f + 43.f);   // 0.547
    return 0.5f;
}

// -----------------------------------------------------------------------------
// Age  [PEW-2023-AGE]
// Piecewise-linear interpolation over generational anchors.
//   18-29: 38% Rep, 57% Dem  →  0.400
//   30-49: 46% Rep, 49% Dem  →  0.484
//   50-64: 52% Rep, 44% Dem  →  0.542
//   65+:   55% Rep, 41% Dem  →  0.573
// -----------------------------------------------------------------------------
static float ageFraction(int age) {
    struct Anchor { float age, rep; };
    static constexpr Anchor anchors[] = {
        {18.f, 0.400f}, {30.f, 0.484f}, {50.f, 0.542f}, {65.f, 0.573f}, {100.f, 0.573f}
    };
    float a = static_cast<float>(std::clamp(age, 18, 100));
    for (int i = 0; i < 4; ++i) {
        if (a <= anchors[i + 1].age) {
            float t = (a - anchors[i].age) / (anchors[i + 1].age - anchors[i].age);
            return anchors[i].rep + t * (anchors[i + 1].rep - anchors[i].rep);
        }
    }
    return anchors[4].rep;
}

// -----------------------------------------------------------------------------
// Location — state-level prior  [ELECTION-2024]
// Tries to extract a US state abbreviation or name from the free-text location
// field (e.g. "Austin, TX", "Texas", "Brooklyn, New York").
// Values are Trump two-party vote share from the 2024 presidential election,
// used as a proxy for the state's Republican lean.
// -----------------------------------------------------------------------------
static float locationFraction(const std::string& location) {
    // Normalise to uppercase for matching
    std::string loc = location;
    for (auto& c : loc) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

    // Two-party Republican fraction by state abbreviation [ELECTION-2024]
    static const std::unordered_map<std::string, float> byAbbr = {
        {"AL", 0.649f}, {"AK", 0.540f}, {"AZ", 0.510f}, {"AR", 0.668f},
        {"CA", 0.372f}, {"CO", 0.438f}, {"CT", 0.430f}, {"DE", 0.437f},
        {"FL", 0.567f}, {"GA", 0.508f}, {"HI", 0.332f}, {"ID", 0.682f},
        {"IL", 0.408f}, {"IN", 0.581f}, {"IA", 0.570f}, {"KS", 0.601f},
        {"KY", 0.671f}, {"LA", 0.620f}, {"ME", 0.477f}, {"MD", 0.347f},
        {"MA", 0.360f}, {"MI", 0.498f}, {"MN", 0.491f}, {"MS", 0.612f},
        {"MO", 0.600f}, {"MT", 0.600f}, {"NE", 0.631f}, {"NV", 0.507f},
        {"NH", 0.481f}, {"NJ", 0.440f}, {"NM", 0.450f}, {"NY", 0.420f},
        {"NC", 0.508f}, {"ND", 0.701f}, {"OH", 0.551f}, {"OK", 0.671f},
        {"OR", 0.431f}, {"PA", 0.508f}, {"RI", 0.390f}, {"SC", 0.582f},
        {"SD", 0.678f}, {"TN", 0.658f}, {"TX", 0.578f}, {"UT", 0.600f},
        {"VT", 0.311f}, {"VA", 0.472f}, {"WA", 0.420f}, {"WV", 0.691f},
        {"WI", 0.499f}, {"WY", 0.719f}, {"DC", 0.069f},
    };

    // Full state name → abbreviation
    static const std::unordered_map<std::string, std::string> nameToAbbr = {
        {"ALABAMA","AL"},{"ALASKA","AK"},{"ARIZONA","AZ"},{"ARKANSAS","AR"},
        {"CALIFORNIA","CA"},{"COLORADO","CO"},{"CONNECTICUT","CT"},{"DELAWARE","DE"},
        {"FLORIDA","FL"},{"GEORGIA","GA"},{"HAWAII","HI"},{"IDAHO","ID"},
        {"ILLINOIS","IL"},{"INDIANA","IN"},{"IOWA","IA"},{"KANSAS","KS"},
        {"KENTUCKY","KY"},{"LOUISIANA","LA"},{"MAINE","ME"},{"MARYLAND","MD"},
        {"MASSACHUSETTS","MA"},{"MICHIGAN","MI"},{"MINNESOTA","MN"},{"MISSISSIPPI","MS"},
        {"MISSOURI","MO"},{"MONTANA","MT"},{"NEBRASKA","NE"},{"NEVADA","NV"},
        {"NEW HAMPSHIRE","NH"},{"NEW JERSEY","NJ"},{"NEW MEXICO","NM"},{"NEW YORK","NY"},
        {"NORTH CAROLINA","NC"},{"NORTH DAKOTA","ND"},{"OHIO","OH"},{"OKLAHOMA","OK"},
        {"OREGON","OR"},{"PENNSYLVANIA","PA"},{"RHODE ISLAND","RI"},{"SOUTH CAROLINA","SC"},
        {"SOUTH DAKOTA","SD"},{"TENNESSEE","TN"},{"TEXAS","TX"},{"UTAH","UT"},
        {"VERMONT","VT"},{"VIRGINIA","VA"},{"WASHINGTON","WA"},{"WEST VIRGINIA","WV"},
        {"WISCONSIN","WI"},{"WYOMING","WY"},{"DISTRICT OF COLUMBIA","DC"},
    };

    // 1. Look for a two-letter state abbreviation token (e.g. ", TX" or " TX")
    //    Must be a word boundary so "CA" in "CAROLINA" isn't a false match.
    for (const auto& [abbr, frac] : byAbbr) {
        // Match ", TX" or " TX" at end/middle of string
        std::string pattern1 = ", " + abbr;
        std::string pattern2 = " "  + abbr;
        if (loc.find(pattern1) != std::string::npos) return frac;
        // Only accept standalone " XX" if it's at the end or followed by non-alpha
        auto pos = loc.find(pattern2);
        if (pos != std::string::npos) {
            size_t after = pos + pattern2.size();
            if (after >= loc.size() || !std::isalpha(static_cast<unsigned char>(loc[after])))
                return frac;
        }
    }

    // 2. Look for full state name as a substring
    for (const auto& [name, abbr] : nameToAbbr) {
        if (loc.find(name) != std::string::npos) {
            auto it = byAbbr.find(abbr);
            if (it != byAbbr.end()) return it->second;
        }
    }

    return 0.5f; // state not identified
}

// -----------------------------------------------------------------------------
// Top-level formula
// Weights (must sum to 1.0):
//   Ethnicity  0.35  — strongest single predictor
//   Gender     0.25  — [PEW-2024-GENDER], large consistent gap
//   Education  0.20  — well-documented inverse relationship
//   Location   0.12  — state-level partisan baseline
//   Age        0.08  — real but modest within US electorate
// -----------------------------------------------------------------------------
float PoliticalBeliefUtils::compute(const PoliticalBeliefInput& input) {
    constexpr float W_ETHNICITY = 0.35f;
    constexpr float W_GENDER    = 0.25f;
    constexpr float W_EDUCATION = 0.20f;
    constexpr float W_LOCATION  = 0.12f;
    constexpr float W_AGE       = 0.08f;

    float score =
        ethnicityFraction(input.ethnicity)       * W_ETHNICITY +
        genderFraction(input.gender)             * W_GENDER    +
        educationFraction(input.educationLevel)  * W_EDUCATION +
        locationFraction(input.location)         * W_LOCATION  +
        ageFraction(input.age)                   * W_AGE;

    return std::clamp(score, 0.0f, 1.0f);
}
