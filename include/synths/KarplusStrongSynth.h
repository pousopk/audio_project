#pragma once
#include <array>
#include <cmath>
#include <vector>
#include <string>
#include "../../strumming/StrummingPattern.h"

class KarplusStrongSynth {
public:
    KarplusStrongSynth();
    void setSampleRate(unsigned int rate);
    void setVolume(double volume);
    void setDecay(double decay);
    void setExciterBrightness(double brightness);
    void setPickTransient(double amount);
    void setDispersion(double amount);
    void setDrive(double drive);
    void setBodyResonance(double amount);
    void setSympatheticResonance(double amount);
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
    double decay_ = 0.992;
    double lossCutoffNorm_ = 0.14;
    double dispersionCoeff_ = 0.08;
    double nonlinearityDrive_ = 0.8;
    double transientAmount_ = 0.20;
    double sympatheticAmount_ = 0.03;

    // Body resonance EQ bank (shared coefficients, per-voice states)
    static constexpr int kBodyModes = 3;
    struct Biquad {
        double b0 = 0.0;
        double b1 = 0.0;
        double b2 = 0.0;
        double a1 = 0.0;
        double a2 = 0.0;
    };
    std::array<Biquad, kBodyModes> bodyFilters_{};
    std::array<double, kBodyModes> bodyModeGains_ = {0.55, 0.30, 0.18};
    double bodyGain_ = 0.55;

    static constexpr int kNumStrings = 6;
    struct Voice {
        std::vector<double> delayLine;
        int writeIndex = 0;
        double freq = 440.0;
        int lastAttackSample = 0;
        bool plucked = false;
        double lossState = 0.0;
        double dispPrevIn = 0.0;
        double dispPrevOut = 0.0;
        std::array<double, kBodyModes> bodyX1{};
        std::array<double, kBodyModes> bodyX2{};
        std::array<double, kBodyModes> bodyY1{};
        std::array<double, kBodyModes> bodyY2{};
    } voices_[kNumStrings];

    double midiToFreq(int midi) const { return 440.0 * std::pow(2.0, (midi - 69) / 12.0); }
    void updateBodyResonanceCoeffs();
    void injectSympathetic(int drivingString, double drivingFreq, const std::vector<int>& shapeFrets);
    void pluck(int string, double freq, int absSample);
    double nextSample(Voice& v);
};
