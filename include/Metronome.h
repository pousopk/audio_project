#pragma once
#include <RtAudio.h>
#include <atomic>
#include <thread>
#include <csignal>

class Metronome {
public:
        int getCurrentBeatIndex() const { return state_.beatIndex; }
    // Fill a buffer with metronome audio (for mixing)
    void fillBuffer(float* out, unsigned int nFrames);
    Metronome();
    ~Metronome();
    void reset();

    void setBpm(double bpm);
    void setBeatsPerBar(int beats);
    void setTimeSignatureDenominator(int denom);
    void setSubdivisions(int subdivisions);
    void setVolume(double volume);
    void setClickFreqStrong(double freq);
    void setClickFreqWeak(double freq);
    void setClickDuration(double sec);

    double getBpm() const;
    int getBeatsPerBar() const;
    int getTimeSignatureDenominator() const;
    int getSubdivisions() const;
    double getVolume() const;
    double getClickFreqStrong() const;
    double getClickFreqWeak() const;
    double getClickDuration() const;

    bool start();
    void stop();
    bool isRunning() const;

    static void printUsage(const char* progName);
    static double parseDoubleArg(int argc, char** argv, const char* flag, double defaultValue);
    static int parseIntArg(int argc, char** argv, const char* flag, int defaultValue);

private:
    struct State {
        double sampleRate = 48000.0;
        std::atomic<double> bpm{120.0};
        double volume = 0.7;
        double phase = 0.0;
        int samplesUntilBeat = 0;
        int clickSamplesRemaining = 0;
        int beatIndex = 0;
        int beatsPerBar = 4;
        int timeSigDenominator = 4; // 4 = quarter note, 8 = eighth note, etc.
        int subdivisions = 1; // new: sub-beats per beat
        int subdivisionIndex = 0; // new: current sub-beat
        double clickFreqStrong = 1760.0;
        double clickFreqWeak = 1320.0;
        double clickDurationSec = 0.02;
    };
    // ...existing code...
    // (removed duplicate private declarations)
    // ...existing code...

    static int audioCallback(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void* userData);
    static void handleSignal(int);

    State state_;
    RtAudio dac_;
    std::atomic<bool> running_;
};
