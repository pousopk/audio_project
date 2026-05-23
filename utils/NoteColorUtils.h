#pragma once
#include <map>
#include <string>
#include <utility>

struct NoteColor {
    int r;
    int g;
    int b;
};

/**
 * @brief Returns the interval name and color for a note relative to a root note.
 * @param note The note name.
 * @param rootNote The root note name.
 * @return Pair of interval name and RGB color.
 */
inline std::pair<std::string, NoteColor> getIntervalAndColor(const std::string& note, const std::string& rootNote) {
    static const std::map<std::string, int> noteIndices = {
        {"C",0}, {"C#",1}, {"Db",1}, {"D",2}, {"D#",3}, {"Eb",3}, {"E",4}, {"F",5},
        {"F#",6}, {"Gb",6}, {"G",7}, {"G#",8}, {"Ab",8}, {"A",9}, {"A#",10}, {"Bb",10}, {"B",11}
    };

    const auto rootIt = noteIndices.find(rootNote);
    const auto noteIt = noteIndices.find(note);
    int rootIdx = (rootIt != noteIndices.end()) ? rootIt->second : 0;
    int noteIdx = (noteIt != noteIndices.end()) ? noteIt->second : 0;
    int interval = (noteIdx - rootIdx + 12) % 12;

    switch (interval) {
        case 0:  return {"1", {255, 70, 70}};      // Root - Red
        case 1:  return {"b2", {128, 128, 128}};
        case 2:  return {"2", {211, 211, 211}};
        case 3:  return {"b3", {100, 100, 255}};   // Minor 3rd - Dark Blue
        case 4:  return {"3", {150, 150, 255}};    // Major 3rd - Bright Blue
        case 5:  return {"4", {211, 211, 211}};
        case 6:  return {"b5", {128, 128, 128}};
        case 7:  return {"5", {70, 180, 70}};      // 5th - Green
        case 8:  return {"#5", {128, 128, 128}};
        case 9:  return {"6", {211, 211, 211}};
        case 10: return {"b7", {200, 100, 200}};   // Minor 7th - Dark Purple
        case 11: return {"7", {255, 150, 255}};    // Major 7th - Bright Purple
        default: return {"", {0, 0, 0}};
    }
}
