#include "../strumming/StrummingPattern.h"
void resetChordSyncState();
#pragma once
#include <RtAudio.h>
#include "ChordProgression.h"
#include <vector>
#include <atomic>
#include <functional>


class ChordAudioEngine {
public:
    ChordAudioEngine();
    ~ChordAudioEngine();

    void setProgression(const ChordProgression& progression);
    void setBpm(double bpm);
    void setSampleRate(unsigned int rate);
    void start();
    void stop();
    bool isRunning() const;

    void setMetronome(class Metronome* m);

    // Chord change callback: called with chord name at every chord change
    void setChordChangeCallback(std::function<void(const std::string&)> cb) { chordChangeCallback = std::move(cb); }

    void setVolume(double v) { chordVolume = v; }
    double getVolume() const { return chordVolume; }

    // Strumming pattern selection
    void setStrummingPattern(const StrummingPattern& pattern) { strummingPattern = pattern; }
    const StrummingPattern& getStrummingPattern() const { return strummingPattern; }

private:
    std::function<void(const std::string&)> chordChangeCallback;
    static int audioCallback(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void* userData);
    void fillBuffer(float* out, unsigned int nFrames);

    RtAudio dac;
    unsigned int sampleRate = 48000;
    double bpm = 120.0;
    std::atomic<bool> running{false};
    ChordProgression progression;
    int currentChordIndex = 0;
    int samplesLeftInChord = 0;
    class Metronome* metronome = nullptr;
    double chordVolume = 1.0;
    StrummingPattern strummingPattern;
};
