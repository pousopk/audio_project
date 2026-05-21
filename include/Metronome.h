#pragma once
#include <RtAudio.h>
#include <atomic>
#include <thread>
#include <csignal>
#include <functional>

/**
 * @brief Real-time metronome with audio output and beat callbacks.
 */
class Metronome {
public:
    /** @brief Get the current beat index. */
    int getCurrentBeatIndex() const { return state_.beatIndex; }
    /** @brief Fill a buffer with metronome audio (for mixing). */
    void fillBuffer(float* out, unsigned int nFrames);
    /** @brief Construct a Metronome. */
    Metronome();
    /** @brief Destructor. */
    ~Metronome();
    /** @brief Reset the metronome state. */
    void reset();

    /** @brief Set the tempo in BPM. */
    void setBpm(double bpm);
    /** @brief Set the number of beats per bar. */
    void setBeatsPerBar(int beats);
    /** @brief Set the time signature denominator. */
    void setTimeSignatureDenominator(int denom);
    /** @brief Set the number of subdivisions per beat. */
    void setSubdivisions(int subdivisions);
    /** @brief Set the output volume. */
    void setVolume(double volume);
    /** @brief Set the frequency for strong beats. */
    void setClickFreqStrong(double freq);
    /** @brief Set the frequency for weak beats. */
    void setClickFreqWeak(double freq);
    /** @brief Set the click duration in seconds. */
    void setClickDuration(double sec);

    /**
     * @brief Set a callback for beat changes.
     * @param cb Callback function called with beat index at every beat change.
     */
    void setBeatCallback(std::function<void(int)> cb) { beatCallback = std::move(cb); }

    double getBpm() const;
    int getBeatsPerBar() const;
    int getTimeSignatureDenominator() const;
    int getSubdivisions() const;
    double getVolume() const;
    double getClickFreqStrong() const;
    double getClickFreqWeak() const;
    double getClickDuration() const;

    /** @brief Start the metronome. */
    bool start();
    /** @brief Stop the metronome. */
    void stop();
    /** @brief Check if the metronome is running. */
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
        int beatIndex = -1;
        int beatsPerBar = 4;
        int timeSigDenominator = 4; // 4 = quarter note, 8 = eighth note, etc.
        int subdivisions = 1; // new: sub-beats per beat
        int subdivisionIndex = 0; // new: current sub-beat
        double clickFreqStrong = 1760.0;
        double clickFreqWeak = 1320.0;
        double clickDurationSec = 0.02;
    };

    static int audioCallback(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void* userData);
    static void handleSignal(int);

    std::function<void(int)> beatCallback;
    State state_;
    RtAudio dac_;
    std::atomic<bool> running_;
};
