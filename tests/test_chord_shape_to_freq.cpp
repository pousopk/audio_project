#include <iostream>
#include <vector>
#include <array>
#include <cmath>
#include "GuitarChordShapeTest.h"

// Standard tuning MIDI for each string
static const std::array<int, 6> stringMidi = {40, 45, 50, 55, 59, 64};

// Helper: MIDI to frequency
static double midiToFreq(int midi) {
    return 440.0 * std::pow(2.0, (midi - 69) / 12.0);
}

void printChordShapeFrequencies(const GuitarChordShapeTest& shape) {
    std::cout << "Chord: " << shape.name << std::endl;
    for (int s = 0; s < 6; ++s) {
        int fret = shape.frets.size() > s ? shape.frets[s] : -1;
        if (fret == -1) {
            std::cout << "String " << (6-s) << ": muted" << std::endl;
        } else {
            int midi = stringMidi[s] + fret;
            double freq = midiToFreq(midi);
            std::cout << "String " << (6-s) << ": fret " << fret << ", MIDI " << midi << ", freq " << freq << " Hz" << std::endl;
        }
    }
}

int main() {
    // C major open chord
    GuitarChordShapeTest cShape;
    cShape.name = "C";
    cShape.frets = {0, 1, 0, 2, 3, -1};
    printChordShapeFrequencies(cShape);
    // G major open chord
    GuitarChordShapeTest gShape;
    gShape.name = "G";
    gShape.frets = {3, 0, 0, 0, 2, 3};
    printChordShapeFrequencies(gShape);
    // D major open chord
    GuitarChordShapeTest dShape;
    dShape.name = "D";
    dShape.frets = {2, 3, 2, 0, -1, -1};
    printChordShapeFrequencies(dShape);
    return 0;
}
