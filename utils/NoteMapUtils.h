#pragma once
#include <QString>
#include <vector>
#include <array>
#include <map>
#include "../widgets/NoteMapFretboardWidget.h"

/**
 * @brief Converts a note name (e.g., "E", "G#") to a MIDI number (octave 2 by default).
 * @param note The note name.
 * @return MIDI note number.
 */
int noteNameToMidi(const QString& note);

/**
 * @brief Generates a NoteMap for the given chord notes.
 * @param notes List of note names (e.g., {"E", "G#", "B"}).
 * @param chordName Optional chord name for labeling.
 * @return NoteMap structure.
 */
NoteMap makeNoteMap(const std::vector<QString>& notes, const QString& chordName = "");
