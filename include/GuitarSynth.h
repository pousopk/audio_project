#pragma once

#include <vector>
#include <array>
#include <string>
#include "../strumming/StrummingPattern.h"

class GuitarSynth {
public:
    GuitarSynth();

    void setSampleRate(unsigned int rate);
    void setVolume(double volume);
    void setFilterCoefficient(double coeff);
    void setDecayFactor(double factor);

    // Process a block of audio for a given chord shape and strumming pattern
    void process(float* out,
                 unsigned int nFrames,
                 const std::vector<int>& shapeFrets,
                 const StrummingPattern& strummingPattern,
                 int samplesPerMeasure,
                 int measureSampleCounter,
                 int beatsPerBar,
                 int samplesPerBeat);

private:
    void initialize();

    unsigned int sampleRate_ = 48000;
    double volume_ = 1.0;
    double filterCoefficient_ = 0.7; // For brightness (0.0 = darkest, 1.0 = brightest), higher is brighter
    double decayFactor_ = 0.99;      // For decay (0.0 = fastest, 1.0 = slowest)

    // Synthesis state
    bool initialized_ = false;
    std::array<double, 6> detuneRatios_;
    std::array<int, 6> lastAttackSample_ = {0,0,0,0,0,0};

    // Karplus-Strong state
    std::array<std::vector<double>, 6> delayLines_;
    std::array<int, 6> writeIndices_ = {0,0,0,0,0,0};
    std::array<double, 6> filterState_ = {0,0,0,0,0,0}; // State for the new filter
};