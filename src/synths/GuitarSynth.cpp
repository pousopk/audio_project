#include "GuitarSynth.h"
#include <cmath>
#include <iostream>
#include <vector>
#include <array>
#include <algorithm>

// Helpers from ChordAudioEngine, now local to synth
static const std::array<int, 6> stringMidi = {40, 45, 50, 55, 59, 64};
static int getMidiForStringFret(int string, int fret) {
    return stringMidi[string] + fret;
}
static double midiToFreq(int midi) {
    return 440.0 * std::pow(2.0, (midi - 69) / 12.0);
}

GuitarSynth::GuitarSynth() {
    initialize();
}

void GuitarSynth::setSampleRate(unsigned int rate) {
    sampleRate_ = rate;
}

void GuitarSynth::setVolume(double volume) {
    volume_ = volume;
}

void GuitarSynth::setFilterCoefficient(double coeff) {
    filterCoefficient_ = std::clamp(coeff, 0.0, 1.0);
}

void GuitarSynth::setDecayFactor(double factor) {
    decayFactor_ = std::clamp(factor, 0.0, 1.0);
}

void GuitarSynth::initialize() {
    if (!initialized_) {
        const double detuneCents = 4.0;
        for (int s = 0; s < 6; ++s) {
            double cents = ((double)rand() / RAND_MAX - 0.5) * 2 * detuneCents;
            detuneRatios_[s] = std::pow(2.0, cents / 1200.0);
        }
        initialized_ = true;
    }
}

void GuitarSynth::process(float* out,
                        unsigned int nFrames,
                        const std::vector<int>& shapeFrets,
                        const StrummingPattern& strummingPattern,
                        int samplesPerMeasure,
                        int measureSampleCounter,
                        int beatsPerBar,
                        int samplesPerBeat)
{
    // Synthesis parameters
    const double amp = 1.0 * volume_; // Karplus-Strong is naturally loud
    const double releaseSamplesShort = 0.05 * sampleRate_; // For short, muted strums

    // Use the strumming pattern for this chord
    const auto& events = strummingPattern.events;

    // Precompute all string attack times for this measure
    std::vector<std::vector<int>> stringAttackSchedule(6);
    for (const auto& ev : events) {
        int evSample = static_cast<int>(ev.position * samplesPerMeasure);
        if (evSample < 0 || evSample >= samplesPerMeasure) continue;
        StrumType type = ev.type;
        int numNotes = ev.numNotes;
        std::vector<int> stringOrder;
        if (type == StrumType::Down || type == StrumType::ArpeggioAsc || type == StrumType::DownAltBass) {
            for (int s = 0; s < 6; ++s) if ((int)shapeFrets.size() > s && shapeFrets[s] != -1) stringOrder.push_back(s);
        } else if (type == StrumType::Up || type == StrumType::ArpeggioDesc) {
            for (int s = 5; s >= 0; --s) if ((int)shapeFrets.size() > s && shapeFrets[s] != -1) stringOrder.push_back(s);
        }
        // For alternating bass, find the second bass note
        if (type == StrumType::DownAltBass && stringOrder.size() > 1) {
            int altBassString = stringOrder[1];
            stringOrder = {altBassString};
        }
        if (numNotes > 0 && numNotes < (int)stringOrder.size()) {
            if (type == StrumType::Up || type == StrumType::ArpeggioDesc) {
                stringOrder.erase(stringOrder.begin(), stringOrder.end() - numNotes);
            } else {
                stringOrder.resize(numNotes);
            }
        }
        if (type == StrumType::ArpeggioAsc || type == StrumType::ArpeggioDesc) {
            if (!stringOrder.empty()) {
                int notes = (int)stringOrder.size();
                for (int i = 0; i < beatsPerBar; ++i) {
                    int s = stringOrder[i % notes];
                    int beatSample = evSample + i * samplesPerBeat;
                    if (beatSample < samplesPerMeasure)
                        stringAttackSchedule[s].push_back(beatSample);
                }
            }
        } else {
            double strumDuration = 0.04;
            int strumSamples = std::max(1, static_cast<int>(strumDuration * sampleRate_));
            for (size_t i = 0; i < stringOrder.size(); ++i) {
                int s = stringOrder[i];
                int attackSample = evSample + (int)(i * (strumSamples / std::max(1, (int)stringOrder.size()-1)));
                if (attackSample < samplesPerMeasure)
                    stringAttackSchedule[s].push_back(attackSample);
            }
        }
    }

    for (unsigned int f = 0; f < nFrames; ++f) {
        int absSample = measureSampleCounter + f;
        // Synthesize sample
        double sample = 0.0;
        for (int s = 0; s < 6; ++s) {
            int fret = (int)shapeFrets.size() > s ? shapeFrets[s] : -1;
            if (fret == -1) continue;

            // On pluck, initialize the delay line with noise
            bool pluck_event = false;
            for (int attack_sample : stringAttackSchedule[s]) {
                if (attack_sample == absSample % samplesPerMeasure) {
                    pluck_event = true;
                    lastAttackSample_[s] = absSample;
                    break;
                }
            }
            if (pluck_event) {
                double freq = midiToFreq(getMidiForStringFret(s, fret)) * detuneRatios_[s];
                int delayLength = static_cast<int>(sampleRate_ / freq);
                if (delayLength > 0) {
                    delayLines_[s].assign(delayLength, 0.0);
                    for (int i = 0; i < delayLength; ++i) {
                        delayLines_[s][i] = (((double)rand() / RAND_MAX) * 2.0 - 1.0) * 0.5; // Softer attack
                    }
                    writeIndices_[s] = 0;
                }
            }

            if (delayLines_[s].empty()) continue;

            // Karplus-Strong algorithm
            int p = delayLines_[s].size();
            int readIndex = writeIndices_[s];
            int nextReadIndex = (readIndex + 1) % p;

            double current_val = delayLines_[s][readIndex];
            double next_val = delayLines_[s][nextReadIndex];

            // A more advanced filter to reduce "chimey" sound.
            // It combines a low-pass filter with an all-pass to add inharmonicity.
            double filtered_val = (current_val + filterState_[s]) * 0.5;
            double new_val = decayFactor_ * filtered_val;
            filterState_[s] = new_val;

            // Apply a simple envelope for short patterns to mute them
            int attackPos = absSample - lastAttackSample_[s];
            bool isSustainedPattern = strummingPattern.name.find("Sustain") != std::string::npos;
            if (!isSustainedPattern && attackPos > releaseSamplesShort) {
                new_val = 0; // Mute short patterns after release time
            }

            delayLines_[s][writeIndices_[s]] = new_val;
            writeIndices_[s] = (writeIndices_[s] + 1) % p;

            sample += new_val;
        }

        // Mix and clip
        out[f] = static_cast<float>(std::tanh(amp * sample / 6.0)); // Divide by 6 to prevent clipping
    }
}
