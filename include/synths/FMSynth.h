#pragma once
#include <cmath>
#include <vector>
#include <string>
#include "../../strumming/StrummingPattern.h"

class FMSynth {
public:
    FMSynth();
    void setSampleRate(unsigned int rate);
    void setVolume(double volume);
    void setModRatio(double ratio);
    void setModIndex(double index);
    void setEnvelope(double attack, double decay, double sustain, double release);
    void process(float* out,
                unsigned int nFrames,
                const std::vector<int>& shapeFrets,
                const StrummingPattern& strummingPattern,
                int samplesPerMeasure,
                int measureSampleCounter,
                int beatsPerBar,
                int samplesPerBeat);
private:
    unsigned int sampleRate_ = 48000;
    double volume_ = 1.0;

    static constexpr int kNumStrings = 6;
    struct Voice {
        double carrierPhase = 0.0;
        double modPhase = 0.0;
        double carrierFreq = 440.0;
        double modRatio = 2.0;
        double modIndex = 3.0;
        double env = 0.0;
        int envStage = 0;
        int lastAttackSample = 0;
    } voices_[kNumStrings];

    double attack_ = 0.01;
    double decay_ = 0.05;
    double sustain_ = 0.7;
    double release_ = 0.1;

    double midiToFreq(int midi) const { return 440.0 * std::pow(2.0, (midi - 69) / 12.0); }
    void noteOn(int string, double freq, int absSample);
    double nextEnvelope(Voice& v, int absSample);
    double nextSample(Voice& v);
};
