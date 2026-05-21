#pragma once
#include <QString>
#include <QColor>
#include <vector>
#include <QMap>
#include <map>

/**
 * @brief Returns the interval name and color for a note relative to a root note.
 * @param note The note name.
 * @param rootNote The root note name.
 * @return Pair of interval name and QColor.
 */
inline std::pair<QString, QColor> getIntervalAndColor(const QString& note, const QString& rootNote) {
    static const QMap<QString, int> noteIndices = {
        {"C",0}, {"C#",1}, {"Db",1}, {"D",2}, {"D#",3}, {"Eb",3}, {"E",4}, {"F",5},
        {"F#",6}, {"Gb",6}, {"G",7}, {"G#",8}, {"Ab",8}, {"A",9}, {"A#",10}, {"Bb",10}, {"B",11}
    };

    int rootIdx = noteIndices.value(rootNote, 0);
    int noteIdx = noteIndices.value(note, 0);
    int interval = (noteIdx - rootIdx + 12) % 12;

    switch (interval) {
        case 0:  return {"1", QColor(255, 70, 70)};   // Root - Red
        case 1:  return {"b2", Qt::darkGray};
        case 2:  return {"2", Qt::lightGray};
        case 3:  return {"b3", QColor(100, 100, 255)}; // Minor 3rd - Dark Blue
        case 4:  return {"3", QColor(150, 150, 255)}; // Major 3rd - Bright Blue
        case 5:  return {"4", Qt::lightGray};
        case 6:  return {"b5", Qt::darkGray};
        case 7:  return {"5", QColor(70, 180, 70)};   // 5th - Green
        case 8:  return {"#5", Qt::darkGray};
        case 9:  return {"6", Qt::lightGray};
        case 10: return {"b7", QColor(200, 100, 200)}; // Minor 7th - Dark Purple
        case 11: return {"7", QColor(255, 150, 255)}; // Major 7th - Bright Purple
        default: return {"", Qt::black};
    }
}
