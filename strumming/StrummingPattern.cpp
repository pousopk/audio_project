#include "StrummingPattern.h"

const std::vector<StrummingPattern> kStrummingPatterns = {
    // General Patterns
    {"Sustain: Whole Note", {{0.0, StrumType::Down, 0}}}, // Strum on 1 and hold
    {"Eighths: Down-Up", {{0.0, StrumType::Down, 0}, {0.5, StrumType::Up, 0}}},
    {"Arpeggio Asc", {{0.0, StrumType::ArpeggioAsc, 0}}},
    {"Arpeggio Desc", {{0.0, StrumType::ArpeggioDesc, 0}}},

    // Bluegrass Patterns (4/4)
    {"Bluegrass: Alt Bass Boom-Chick", {
        {0.0,  StrumType::Down, 1},       // Boom on beat 1 (root)
        {0.25, StrumType::Down, 4},       // Chick on beat 2 (treble strings)
        {0.5,  StrumType::DownAltBass, 1},// Boom on beat 3 (alternate bass)
        {0.75, StrumType::Down, 4}        // Chick on beat 4 (treble strings)
    }},
    {"Bluegrass: Bass-Strum", {
        {0.0,  StrumType::Down, 1}, // Bass on 1
        {0.25, StrumType::Down, 0}, // Strum on 2
        {0.5,  StrumType::Down, 1}, // Bass on 3
        {0.75, StrumType::Down, 0}  // Strum on 4
    }},
    {"Bluegrass: Bass-Strum-Up", {
        {0.0,  StrumType::Down, 1}, // Bass on 1
        {0.25, StrumType::Down, 0}, // Strum on 2
        {0.5,  StrumType::Down, 1}, // Bass on 3
        {0.75, StrumType::Up, 0}    // Up-Strum on 4
    }},

    // Bluegrass Pattern (3/4 - Waltz)
    {"Bluegrass: Waltz", {
        {0.0,       StrumType::Down, 1}, // Bass on 1
        {1.0/3.0,   StrumType::Down, 4}, // Strum on 2
        {2.0/3.0,   StrumType::Down, 4}  // Strum on 3
    }},
};
