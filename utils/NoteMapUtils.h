#pragma once
#include <QString>
#include <vector>
#include <array>
#include <map>
#include "../widgets/NoteMapFretboardWidget.h"

// Converts note name (e.g., "E", "G#") to MIDI number (octave 2 by default)
int noteNameToMidi(const QString& note);

// Generates a NoteMap for the given chord notes (e.g., {"E", "G#", "B"})
NoteMap makeNoteMap(const std::vector<QString>& notes, const QString& chordName = "");
