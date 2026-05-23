
#pragma once
#include <vector>
#include <string>

/**
 * @brief Types of strumming or picking events.
 */
enum class StrumType {
    Down,
    Up,
    ArpeggioAsc,
    ArpeggioDesc,
    DownAltBass // For alternating bass notes
};

/**
 * @brief Represents a single strum or picking event in a pattern.
 */
struct StrumEvent {
    double position; ///< Position in bar (0.0-1.0)
    StrumType type; ///< Type of strum or arpeggio
    int numNotes; ///< For arpeggios: how many notes to play (0 = all)
};

/**
 * @brief Represents a strumming pattern for guitar.
 */
struct StrummingPattern {
    std::string name; ///< Name of the strumming pattern
    std::vector<StrumEvent> events; ///< Sequence of strum events
};

// Some common patterns (to be filled in .cpp)
extern const std::vector<StrummingPattern> kStrummingPatterns;
