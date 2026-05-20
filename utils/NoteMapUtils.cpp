#include "NoteMapUtils.h"

// Standard tuning: E2 A2 D3 G3 B3 E4
static const std::array<QString, 6> stringNotes = {"E", "A", "D", "G", "B", "E"};
static const std::array<int, 6> stringMidi = {40, 45, 50, 55, 59, 64};

int noteNameToMidi(const QString& note) {
    static std::map<QString, int> base = {{"C",0},{"C#",1},{"D",2},{"D#",3},{"E",4},{"F",5},{"F#",6},{"G",7},{"G#",8},{"A",9},{"A#",10},{"B",11}};
    int octave = 2; // default
    QString n = note;
    if (note.length() > 1 && note[note.length()-1].isDigit()) {
        octave = note[note.length()-1].digitValue();
        n = note.left(note.length()-1);
    }
    return base[n] + 12 * octave;
}

NoteMap makeNoteMap(const std::vector<QString>& notes, const QString& chordName) {
    NoteMap map;
    map.noteFrets.resize(6);
    for (int s = 0; s < 6; ++s) {
        int openMidi = stringMidi[s];
        for (int fret = 0; fret <= 24; ++fret) {
            int midi = openMidi + fret;
            int noteClass = midi % 12;
            for (const auto& n : notes) {
                if (noteNameToMidi(n) % 12 == noteClass) {
                    map.noteFrets[s].push_back(fret);
                }
            }
        }
    }
    map.chordName = chordName;
    return map;
}
