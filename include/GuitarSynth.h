#pragma once

#include <vector>
#include <array>
#include <string>
#include "../strumming/StrummingPattern.h"

/**
 * @brief Physical modeling guitar synthesizer using Karplus-Strong algorithm.
 */
class GuitarSynth {
public:
    /** @brief Construct a GuitarSynth. */
    GuitarSynth();

    /** @brief Set the sample rate. */
    void setSampleRate(unsigned int rate);
    /** @brief Set the output volume. */
    void setVolume(double volume);
    /** @brief Set the filter coefficient (brightness). */
    void setFilterCoefficient(double coeff);
    /** @brief Set the decay factor. */
    void setDecayFactor(double factor);

    /**
     * @brief Process a block of audio for a given chord shape and strumming pattern.
     * @param out Output buffer.
     * @param nFrames Number of frames to process.
     * @param shapeFrets Fret positions for each string.
     * @param strummingPattern Strumming pattern to use.
     * @param samplesPerMeasure Samples per measure.
     * @param measureSampleCounter Current sample in measure.
     * @param beatsPerBar Number of beats per bar.
     * @param samplesPerBeat Samples per beat.
     */
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

    unsigned int sampleRate_ = 48000; ///< Sample rate
    double volume_ = 1.0; ///< Output volume
    double filterCoefficient_ = 0.7; ///< Brightness filter coefficient
    double decayFactor_ = 0.99; ///< Decay factor

    // Synthesis state
    bool initialized_ = false;
    std::array<double, 6> detuneRatios_;
    std::array<int, 6> lastAttackSample_ = {0,0,0,0,0,0};

    // Karplus-Strong state
    std::array<std::vector<double>, 6> delayLines_;
    std::array<int, 6> writeIndices_ = {0,0,0,0,0,0};
    std::array<double, 6> filterState_ = {0,0,0,0,0,0}; // State for the new filter
};