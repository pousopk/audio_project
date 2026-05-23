#include "NoteMapUtils.h"

#include <cctype>

// Standard tuning: E2 A2 D3 G3 B3 E4
static const std::array<int, 6> stringMidi = {40, 45, 50, 55, 59, 64};

int noteNameToMidi(const std::string& note) {
    static const std::map<std::string, int> base = {{"C",0},{"C#",1},{"D",2},{"D#",3},{"E",4},{"F",5},{"F#",6},{"G",7},{"G#",8},{"A",9},{"A#",10},{"B",11}};
    int octave = 2; // default
    std::string n = note;
    if (note.length() > 1 && std::isdigit(static_cast<unsigned char>(note.back()))) {
        octave = note.back() - '0';
        n = note.substr(0, note.length() - 1);
    }
    auto it = base.find(n);
    const int baseNote = (it != base.end()) ? it->second : 0;
    return baseNote + 12 * octave;
}

NoteMap makeNoteMap(const std::vector<std::string>& notes, const std::string& chordName) {
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
