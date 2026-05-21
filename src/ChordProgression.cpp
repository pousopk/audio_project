#include "ChordProgression.h"
#include <QString>

void ChordProgression::addChord(const std::string& chord, int measures, int strummingPatternIndex) {
    chords_.append({QString::fromStdString(chord), measures, strummingPatternIndex});
}


void ChordProgression::removeChord(int index) {
    if (index >= 0 && index < chords_.size()) {
        chords_.removeAt(index);
    }
}

const QVector<ChordChange>& ChordProgression::getChords() const {
    return chords_;
}

void ChordProgression::clear() {
    chords_.clear();
}
