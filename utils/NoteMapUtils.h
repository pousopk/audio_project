#pragma once
#include <vector>
#include <string>
#include <array>
#include <map>
#include "../include/NoteMap.h"

/**
 * @brief Converts a note name (e.g., "E", "G#") to a MIDI number (octave 2 by default).
 * @param note The note name.
 * @return MIDI note number.
 */
int noteNameToMidi(const std::string& note);

/**
 * @brief Generates a NoteMap for the given chord notes.
 * @param notes List of note names (e.g., {"E", "G#", "B"}).
 * @param chordName Optional chord name for labeling.
 * @return NoteMap structure.
 */
NoteMap makeNoteMap(const std::vector<std::string>& notes, const std::string& chordName = "");
