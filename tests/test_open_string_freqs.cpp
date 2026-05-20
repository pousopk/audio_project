#include <iostream>
#include <array>
#include <cmath>

// Standard tuning MIDI for each string (low E to high E)
static const std::array<int, 6> stringMidi = {40, 45, 50, 55, 59, 64};
static const char* stringNames[6] = {"E2", "A2", "D3", "G3", "B3", "E4"};

// Helper: MIDI to frequency
static double midiToFreq(int midi) {
    return 440.0 * std::pow(2.0, (midi - 69) / 12.0);
}

int main() {
    std::cout << "Open string frequencies (standard tuning):" << std::endl;
    for (int s = 0; s < 6; ++s) {
        int midi = stringMidi[s];
        double freq = midiToFreq(midi);
        std::cout << stringNames[s] << ": MIDI " << midi << ", freq " << freq << " Hz" << std::endl;
    }
    return 0;
}
