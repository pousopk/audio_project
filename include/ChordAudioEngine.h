#include "../strumming/StrummingPattern.h"
#pragma once
#include <RtAudio.h>
#include "ChordProgression.h"
#include <vector>
#include <atomic>
#include <functional>
#include <mutex>
#include <array>
#include "GuitarSynth.h"
#include "effects/FXChain.h"


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

    // Synth parameter controls
    void setReverbMix(float mix);
    void setDelayTime(float time_ms);
    void setDelayFeedback(float feedback);
    void setDelayMix(float mix);
    void setDistortionDrive(float drive);
    void setDistortionMix(float mix);
    void setCompressorThreshold(float threshold_dB);
    void setCompressorRatio(float ratio);
    void setCompressorAttack(float attack_ms);
    void setCompressorRelease(float release_ms);
    void setEQLowGain(float gain_db);
    void setEQMidGain(float gain_db);
    void setEQHighGain(float gain_db);
    void setLimiterThreshold(float threshold_db);
    void setLimiterRelease(float release_ms);
    void setChorusRate(float rate_hz);
    void setChorusDepth(float depth_ms);
    void setChorusMix(float mix);
    void setGateThreshold(float threshold_db);
    void setGateRelease(float release_ms);
    void setFlangerRate(float rate_hz);
    void setFlangerDepth(float depth_ms);
    void setFlangerFeedback(float feedback);
    void setFlangerMix(float mix);
    void setPhaserRate(float rate_hz);
    void setPhaserDepth(float depth);
    void setPhaserFeedback(float feedback);
    void setPhaserMix(float mix);
    void setTremoloRate(float rate_hz);
    void setTremoloDepth(float depth);
    void setFXOrder(const std::vector<std::string>& newOrder);

    // Metering
    float getCompressorGainReductionDB() const;
    std::vector<float> getSpectrumData() const;

    // Strumming pattern selection
    void setStrummingPattern(const StrummingPattern& pattern) { strummingPattern = pattern; }
    const StrummingPattern& getStrummingPattern() const { return strummingPattern; }

private:
    void resetChordSyncState();

    // Synthesis & Sync State (moved from global/static)
    int measureSampleCounter_ = 0;
    int chordMeasureCount_ = 0;
    bool started_ = false;

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
    GuitarSynth synth_;
    FXChain fxChain_;
    int lastChordIndex_ = -1;

    // FFT and Spectrum Analyzer members
    std::vector<float> fft_input_buffer_;
    int fft_input_pos_ = 0;
    std::vector<float> spectrum_magnitudes_;
    mutable std::mutex spectrum_mutex_;

};
