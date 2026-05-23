#pragma once
#include <vector>
#include <string>
#include <cstddef>
#include "ChordChange.h"

/**
 * @brief Manages a sequence of chord changes for a progression.
 */
class ChordProgression {
public:
    /**
     * @brief Add a chord to the progression.
     * @param chord The chord name.
     * @param measures Number of measures to play the chord.
     * @param strummingPatternIndex Index of the strumming pattern.
     */
    void addChord(const std::string& chord, int measures, int strummingPatternIndex);
    /**
     * @brief Remove a chord at the given index.
     * @param index The index to remove.
     */
    void removeChord(int index);
    /**
     * @brief Get the list of chord changes.
     * @return Vector of ChordChange.
     */
    const std::vector<ChordChange>& getChords() const;
    /** @brief Clear the progression. */
    void clear();
private:
    std::vector<ChordChange> chords_; ///< List of chord changes
};
