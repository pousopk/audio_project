#pragma once
#include <vector>
#include <string>

/**
 * @brief Represents a guitar chord shape.
 * @details -1 = muted, 0 = open, 1+ = fret number.
 */
struct GuitarChordShape {
    std::vector<int> frets;    ///< Fret numbers for each string (size = 6)
    std::vector<int> fingers;  ///< Optional finger numbers (0 = none)
    std::string name;          ///< Name of the chord shape
};
