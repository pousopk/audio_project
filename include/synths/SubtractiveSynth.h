#pragma once
#include <cmath>
#include <vector>
#include <string>
#include "../../strumming/StrummingPattern.h"

class SubtractiveSynth {
public:
    SubtractiveSynth();
    void setSampleRate(unsigned int rate);
    void setVolume(double volume);
    void setWaveformSaw(bool saw);
    void setCutoff(double cutoffHz);
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

    enum class Waveform { Saw, Square };
    Waveform waveform_ = Waveform::Saw;

    static constexpr int kNumStrings = 6;
    struct Voice {
        double phase = 0.0;
        double freq = 0.0;
        double env = 0.0;
        int envStage = 0; // 0=idle,1=attack,2=decay,3=sustain,4=release
        double filterMem = 0.0;
        int lastAttackSample = 0;
    } voices_[kNumStrings];

    // Envelope (ADSR)
    double attack_ = 0.01;
    double decay_ = 0.05;
    double sustain_ = 0.7;
    double release_ = 0.1;

    // Filter
    double cutoff_ = 2000.0;

    // Utility
    double midiToFreq(int midi) const { return 440.0 * std::pow(2.0, (midi - 69) / 12.0); }
    void noteOn(int string, double freq, int absSample);
    double nextEnvelope(Voice& v, int absSample);
    double nextOsc(Voice& v);
    double nextFilter(Voice& v, double in);
};
