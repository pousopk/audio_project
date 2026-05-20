#pragma once
#include <QString>
#include <QColor>

// Returns (degree, color) for a note in a chord
inline std::pair<int, QColor> getNoteDegreeAndColor(const QString& note, const std::vector<QString>& chordNotes) {
    // chordNotes[0] = root, [1] = 3rd, [2] = 5th, [3] = 7th (optional)
    int idx = -1;
    for (int i = 0; i < (int)chordNotes.size(); ++i) {
        if (note == chordNotes[i]) { idx = i; break; }
    }
    QColor color = Qt::black;
    switch (idx) {
        case 0: color = Qt::black; break; // root
        case 1: color = QColor(0, 102, 204); break; // 3rd (blue)
        case 2: color = QColor(255, 204, 0); break; // 5th (yellow)
        case 3: color = QColor(204, 0, 0); break; // 7th (red)
        default: color = Qt::gray; break;
    }
    return {idx, color};
}
