#include "ChordProgression.h"

void ChordProgression::addChord(const std::string& chord, int measures, int strummingPatternIndex) {
    chords.push_back({chord, measures, strummingPatternIndex});
}


void ChordProgression::removeChord(int index) {
    if (index >= 0 && index < static_cast<int>(chords.size())) {
        chords.erase(chords.begin() + index);
    }
}

const std::vector<ChordEntry>& ChordProgression::getChords() const {
    return chords;
}

void ChordProgression::clear() {
    chords.clear();
}
