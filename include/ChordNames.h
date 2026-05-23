#pragma once
#include <string>
#include <vector>

/**
 * @brief Returns a list of all supported chord names.
 * @return Vector of chord names.
 */
inline std::vector<std::string> getAllChordNames() {
    // Basic set, can be expanded
    return {
        "C", "Cm", "C7", "Cm7", "Cmaj7",
        "D", "Dm", "D7", "Dm7", "Dmaj7",
        "E", "Em", "E7", "Em7", "Emaj7",
        "F", "Fm", "F7", "Fm7", "Fmaj7",
        "G", "Gm", "G7", "Gm7", "Gmaj7",
        "A", "Am", "A7", "Am7", "Amaj7",
        "B", "Bm", "B7", "Bm7", "Bmaj7"
    };
}
