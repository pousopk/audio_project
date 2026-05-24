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
        {0.25, StrumType::Down, 3},       // Chick on beat 2 (focused chop)
        {0.5,  StrumType::DownAltBass, 1},// Boom on beat 3 (alternate bass)
        {0.75, StrumType::Down, 3}        // Chick on beat 4 (focused chop)
    }},
    {"Bluegrass: Bass-Strum", {
        {0.0,  StrumType::Down, 1}, // Bass on 1
        {0.25, StrumType::Down, 4}, // Full chop on 2
        {0.5,  StrumType::Down, 1}, // Bass on 3
        {0.75, StrumType::Down, 4}  // Full chop on 4
    }},
    {"Bluegrass: Bass-Strum-Up", {
        {0.0,  StrumType::Down, 1}, // Bass on 1
        {0.25, StrumType::Down, 3}, // Short down chop on 2
        {0.5,  StrumType::Down, 1}, // Bass on 3
        {0.75, StrumType::Up, 2},   // Light upstroke on 4
        {0.875, StrumType::Up, 1}   // Extra ghost upstroke
    }},

    // Bluegrass Pattern (3/4 - Waltz)
    {"Bluegrass: Waltz", {
        {0.0,       StrumType::Down, 1}, // Bass on 1
        {1.0/3.0,   StrumType::Down, 4}, // Strum on 2
        {2.0/3.0,   StrumType::Down, 4}  // Strum on 3
    }},
    {"Bluegrass: Train Beat 8ths", {
        {0.0,   StrumType::Down, 1},
        {0.125, StrumType::Up, 1},
        {0.25,  StrumType::Down, 2},
        {0.375, StrumType::Up, 1},
        {0.5,   StrumType::DownAltBass, 1},
        {0.625, StrumType::Up, 1},
        {0.75,  StrumType::Down, 2},
        {0.875, StrumType::Up, 1}
    }},
    {"Bluegrass: Forward Roll (Crosspick)", {
        {0.0,   StrumType::Down, 1},
        {0.125, StrumType::Up, 1},
        {0.25,  StrumType::Down, 2},
        {0.375, StrumType::Up, 1},
        {0.5,   StrumType::DownAltBass, 1},
        {0.625, StrumType::Up, 1},
        {0.75,  StrumType::Down, 2},
        {0.875, StrumType::Up, 1}
    }},
    {"Bluegrass: Hard Chop Drive", {
        {0.0,  StrumType::Down, 1},
        {0.25, StrumType::Down, 4},
        {0.5,  StrumType::DownAltBass, 1},
        {0.75, StrumType::Down, 4}
    }},
    {"Bluegrass: Waltz Alt Bass", {
        {0.0,       StrumType::Down, 1},
        {1.0/3.0,   StrumType::Down, 3},
        {2.0/3.0,   StrumType::DownAltBass, 1}
    }},

    // Math Rock inspired patterns
    {"Math Rock: 7/8 Pick Grid (2+2+3)", {
        {0.0/7.0, StrumType::Down, 1},
        {1.0/7.0, StrumType::Up, 2},
        {2.0/7.0, StrumType::Down, 1},
        {3.0/7.0, StrumType::Up, 2},
        {4.0/7.0, StrumType::Down, 1},
        {5.0/7.0, StrumType::Up, 2},
        {6.0/7.0, StrumType::Down, 3}
    }},
    {"Math Rock: 5/4 Tapped Pulse (3+2)", {
        {0.0/5.0, StrumType::ArpeggioAsc, 3},
        {1.0/5.0, StrumType::Up, 2},
        {2.0/5.0, StrumType::Down, 1},
        {3.0/5.0, StrumType::ArpeggioDesc, 3},
        {4.0/5.0, StrumType::Up, 2}
    }},
    {"Math Rock: Ghosted Upstrokes", {
        {0.0,  StrumType::Down, 2},
        {0.125, StrumType::Up, 1},
        {0.25, StrumType::Down, 3},
        {0.375, StrumType::Up, 1},
        {0.5,  StrumType::DownAltBass, 1},
        {0.625, StrumType::Up, 1},
        {0.75, StrumType::Down, 3},
        {0.875, StrumType::Up, 2}
    }},
    {"Math Rock: Cascading Arpeggios", {
        {0.0, StrumType::ArpeggioAsc, 0},
        {0.5, StrumType::ArpeggioDesc, 0}
    }},
    {"Math Rock: Syncopated Harmonics", {
        {0.0, StrumType::Down, 2},
        {0.1875, StrumType::Up, 2},
        {0.375, StrumType::Down, 1},
        {0.5625, StrumType::Up, 2},
        {0.75, StrumType::DownAltBass, 1},
        {0.9375, StrumType::Up, 3}
    }},
};
