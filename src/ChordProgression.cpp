/**
 * @file ChordProgression.cpp
 * @brief Implements the ChordProgression class for managing a sequence of chords.
 */
#include "ChordProgression.h"

/**
 * @brief Add a chord to the progression.
 * @param chord Name of the chord (std::string).
 * @param measures Number of measures for the chord.
 * @param strummingPatternIndex Index of the strumming pattern.
 */
void ChordProgression::addChord(const std::string& chord, int measures, int strummingPatternIndex) {
    chords_.push_back({chord, measures, strummingPatternIndex});
}


/**
 * @brief Remove a chord from the progression by index.
 * @param index Index of the chord to remove.
 */
void ChordProgression::removeChord(int index) {
    if (index >= 0 && static_cast<std::size_t>(index) < chords_.size()) {
        chords_.erase(chords_.begin() + index);
    }
}

/**
 * @brief Get the list of chords in the progression.
 * @return Reference to vector of ChordChange.
 */
const std::vector<ChordChange>& ChordProgression::getChords() const {
    return chords_;
}

/**
 * @brief Clear the chord progression.
 */
void ChordProgression::clear() {
    chords_.clear();
}
