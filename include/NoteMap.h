#pragma once

#include <string>
#include <vector>

/**
 * @brief Maps notes to fret positions for a chord or scale.
 */
struct NoteMap {
    std::vector<std::vector<int>> noteFrets; ///< For each string, fret numbers where target notes occur
    std::string chordName; ///< Name of the chord or scale label
    std::vector<std::string> chordTones; ///< Notes to highlight as target tones
};
