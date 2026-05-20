#pragma once
#include <vector>
#include <string>

struct ChordEntry {
    std::string chordName; // e.g. "Cmaj7"
    int measures;         // number of measures for this chord
    int strummingPatternIndex = 0; // index into kStrummingPatterns
};

class ChordProgression {
public:
    void addChord(const std::string& chord, int measures, int strummingPatternIndex);
    void removeChord(int index);
    const std::vector<ChordEntry>& getChords() const;
    void clear();
private:
    std::vector<ChordEntry> chords;
};
