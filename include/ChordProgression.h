#pragma once
#include <vector>
#include <string>
#include "ChordProgressionManager.h" // For ChordChange

class ChordProgression {
public:
    void addChord(const std::string& chord, int measures, int strummingPatternIndex);
    void removeChord(int index);
    // Note: We now use QVector<ChordChange> for consistency with the manager
    const QVector<ChordChange>& getChords() const;
    void clear();
private:
    // Using the same data structure as ChordProgressionManager
    QVector<ChordChange> chords_;
};
