#pragma once
#include <string>

/**
 * @brief Represents a chord change in a progression.
 */
struct ChordChange {
    std::string chordName; ///< Name of the chord
    int bars; ///< Number of bars to play this chord
    int strummingPatternIndex = 0; ///< Strumming pattern index
};
