# audio_project

## Folder Structure

- include/   — Public headers (core logic)
- src/       — Core source files (main logic, audio, metronome, progression)
- widgets/   — Qt widget implementations (fretboard, chord UI, etc.)
- utils/     — Utility/helper headers and sources
- tests/     — Test/demo applications

## Building

1. Install Qt5 or Qt6 development libraries (e.g., `qtbase5-dev` on Ubuntu).
2. From the project root:

```
mkdir -p build
cd build
cmake ..
make
```

## Running Demos/Tests

- Main app: `./audio_project`
- Fretboard demo: `./test_fretboard`
- Horizontal fretboard: `./test_horizontal_fretboard`
- Note map: `./test_note_map`

## Project Overview


This project is a modular C++/Qt audio application featuring:

- Real-time metronome with audio output (RtAudio)
- Visual beat indicator (Qt) with strong beat in red, weak beats in blue, others black
- BPM and time signature controls (including dynamic changes)
- Chord progression backing track with additive synthesis
- Per-chord strumming/arpeggio pattern selection and scheduling
- Chord progression UI: add/remove chords, measures, strumming pattern, volume
- Guitar fretboard and note map visualization widgets
- Full-bar, half-bar, and short strum/arpeggio logic (musically accurate)
- All code modularized: include/, src/, widgets/, utils/, tests/
- Comprehensive test harnesses for metronome, chord/strum logic, fretboard
- Example test: `test_metronome_visual` demonstrates perfect sync between metronome audio and visualizer, with automatic time signature changes

## Usage

### Main Application
- Run `./audio_project` for the full metronome and chord backing track app with GUI.

### Tests/Demos
- `./tests/test_metronome_visual` — Metronome and beat visualizer sync test (starts at 160 BPM 4/4, switches to 3/4 and 5/4)
- `./tests/test_fretboard` — Guitar fretboard visualization
- `./tests/test_chord_shape_to_freq`, `./tests/test_open_string_freqs` — Audio/logic tests

## Features Completed

- Modular CMake/Qt/RtAudio project structure
- Real-time metronome with strong/weak beat distinction
- Visual beat indicator (separate window or embedded)
- BPM and time signature controls (UI and programmatic)
- Chord progression input, editing, and playback
- Per-chord strumming/arpeggio pattern selection
- Accurate strumming/arpeggio scheduling and sustain logic
- Guitar chord shape and inversion logic (expandable)
- Fretboard and note map widgets
- All major UI controls: chord root/type, measure count, strumming pattern, volume
- Test harnesses for all major modules

## TODO / In Progress
- Show all notes of a chord on the fretboard (note map)
- Generate all possible chord shapes (inversions) up to fret 24

See the respective folders for details on each module and test.