# audio_project

## Folder Structure

- include/   — Public headers (core logic, effects API)
- include/effects/ — Audio effect interfaces and effect headers
- src/       — Core source files (main logic, audio, metronome, progression)
- src/effects/ — Effect implementations
- widgets/   — Qt widget implementations (fretboard, chord UI, effects UI, etc.)
- utils/     — Utility/helper headers and sources
- tests/     — Test/demo applications and harnesses

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
- Visual beat indicator (Qt) with strong/weak/other beat coloring
- BPM and time signature controls (including dynamic changes)
- Chord progression backing track with additive synthesis
- Per-chord strumming/arpeggio pattern selection and scheduling
- Chord progression UI: add/remove chords, measures, strumming pattern, volume
- Guitar fretboard and note map visualization widgets
- Full-bar, half-bar, and short strum/arpeggio logic (musically accurate)
- Modular effects system: flexible FXChain, with Delay, Reverb, Distortion, Compressor, EQ, Limiter, Chorus, Gate, Flanger, Phaser, Tremolo
- All code modularized: include/, src/, widgets/, utils/, tests/, effects/
- Comprehensive test harnesses for metronome, chord/strum logic, fretboard, and effects
- Example test: `test_metronome_visual` demonstrates perfect sync between metronome audio and visualizer, with automatic time signature changes
## Modular Effects System

The project features a fully modular audio effects architecture:

- All effect interfaces are in `include/effects/` (e.g., AudioEffect.h, FXChain.h)
- Effect implementations are in `src/effects/` (e.g., Delay.cpp, Reverb.cpp, etc.)
- Effects can be chained, reordered, and parameterized at runtime via the FXChain class
- UI widgets for effect control and visualization are in `widgets/`

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
- Modular FXChain with all major audio effects (see above)
- All major UI controls: chord root/type, measure count, strumming pattern, volume, FX controls
- Test harnesses for all major modules and effects

## TODO / In Progress
- Show all notes of a chord on the fretboard (note map)
- Generate all possible chord shapes (inversions) up to fret 24
- Expand effect parameter UI and presets
- Add more advanced musical features and test cases

See the respective folders for details on each module and test.