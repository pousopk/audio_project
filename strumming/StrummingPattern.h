#pragma once
#include <vector>
#include <string>

// StrumType: Down, Up, Arpeggio, etc.
enum class StrumType {
    Down,
    Up,
    ArpeggioAsc,
    ArpeggioDesc,
    DownAltBass // For alternating bass notes
};

struct StrumEvent {
    double position; // Position in bar (0.0-1.0)
    StrumType type;
    int numNotes; // For arpeggios: how many notes to play (0 = all)
};

struct StrummingPattern {
    std::string name;
    std::vector<StrumEvent> events;
};

// Some common patterns (to be filled in .cpp)
extern const std::vector<StrummingPattern> kStrummingPatterns;
