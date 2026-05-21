#pragma once
#include <QVector>
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
     * @return QVector of ChordChange.
     */
    const QVector<ChordChange>& getChords() const;
    /** @brief Clear the progression. */
    void clear();
private:
    QVector<ChordChange> chords_; ///< List of chord changes
};
