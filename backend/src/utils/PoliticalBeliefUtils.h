#pragma once
#include <string>

// Computes a political belief score purely from projected attributes.
// Score range: 0.0 = far left, 1.0 = far right, 0.5 = centre.
//
// Formula is defined by the product owner — see compute() stub.
struct PoliticalBeliefInput {
    std::string gender;          // "male" | "female" | "unknown"
    std::string ethnicity;       // NamSor code, e.g. "W_NL", "HL", "A", "B_NL"
    std::string educationLevel;  // "high_school" | "some_college" | "bachelors" | "masters" | "phd" | "unknown"
    std::string location;        // free-text, e.g. "Austin, TX"
    int         age;
};

class PoliticalBeliefUtils {
public:
    // TODO: replace stub with the real formula.
    static float compute(const PoliticalBeliefInput& input);
};
