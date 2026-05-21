/**
 * @file Metronome.cpp
 * @brief Implements the Metronome class for real-time audio click generation and beat tracking.
 */

#include "Metronome.h"
#include <cmath>
#include <iostream>
#include <cstring>

/**
 * @brief Reset the metronome's internal state to ensure it starts fresh.
 */
void Metronome::reset() {
    state_.beatIndex = -1;
    state_.subdivisionIndex = 0;
    state_.samplesUntilBeat = 0;
    state_.clickSamplesRemaining = 0;
}

/**
 * @brief Fill a buffer with metronome audio (for mixing).
 * @param out Output buffer.
 * @param nFrames Number of frames to fill.
 */
void Metronome::fillBuffer(float* out, unsigned int nFrames) {
    State* state = &state_;
    const double twoPi = 6.28318530717958647692;
    int clickLenSamples = static_cast<int>(state->clickDurationSec * state->sampleRate);
    double noteLength = 4.0 / state->timeSigDenominator;
    double beatDurationSec = (60.0 / state->bpm.load()) * noteLength;
    int samplesPerBeat = static_cast<int>(beatDurationSec * state->sampleRate);
    for (unsigned int i = 0; i < nFrames; ++i) {
        if (state->samplesUntilBeat <= 0) {
            int samplesPerSub = samplesPerBeat / state->subdivisions;
            state->samplesUntilBeat = samplesPerSub;
            state->clickSamplesRemaining = clickLenSamples;
            state->subdivisionIndex++;
            if (state->subdivisionIndex >= state->subdivisions) {
                state->subdivisionIndex = 0;
                state->beatIndex = (state->beatIndex + 1) % state->beatsPerBar;
            }
        }
        float sample = 0.0f;
        if (state->clickSamplesRemaining > 0) {
            bool strongBeat = (state->beatIndex == 0 && state->subdivisionIndex == 0);
            bool subClick = (state->subdivisionIndex != 0);
            double freq;
            if (strongBeat) {
                freq = state->clickFreqStrong;
            } else if (subClick) {
                freq = state->clickFreqWeak;
            } else {
                freq = state->clickFreqWeak;
            }
            double env = 0.0;
            if (clickLenSamples > 0) {
                env = static_cast<double>(state->clickSamplesRemaining) / static_cast<double>(clickLenSamples);
            }
            double volume = state->volume;
            if (subClick) {
                volume *= 0.5;
            }
            sample = static_cast<float>(volume * env * std::sin(state->phase));
            state->phase += twoPi * freq / state->sampleRate;
            if (state->phase > twoPi) {
                state->phase -= twoPi;
            }
            state->clickSamplesRemaining = state->clickSamplesRemaining - 1;
        }
        out[i] = sample;
        state->samplesUntilBeat--;
    }
}

void Metronome::setTimeSignatureDenominator(int denom)
{
    if (denom == 2 || denom == 4 || denom == 8 || denom == 16) {
        state_.timeSigDenominator = denom;
    }
}


int Metronome::getTimeSignatureDenominator() const
{
    return state_.timeSigDenominator;
}
#include "Metronome.h"
#include <iostream>
#include <cstring>
#include <cmath>

std::atomic<bool> globalRunning{true};


Metronome::Metronome()
    : running_(false)
{
    // Constructor body left intentionally empty
}


Metronome::~Metronome()
{
    stop();
}


void Metronome::setBpm(double bpm)
{
    state_.bpm = bpm;
}


void Metronome::setBeatsPerBar(int beats)
{
    state_.beatsPerBar = beats;
}

void Metronome::setSubdivisions(int subdivisions)
{
    if (subdivisions < 1) subdivisions = 1;
    state_.subdivisions = subdivisions;
}

void Metronome::setVolume(double volume)
{
    state_.volume = volume;
}

void Metronome::setClickFreqStrong(double freq)
{
    state_.clickFreqStrong = freq;
}

void Metronome::setClickFreqWeak(double freq)
{
    state_.clickFreqWeak = freq;
}

void Metronome::setClickDuration(double sec)
{
    state_.clickDurationSec = sec;
}

double Metronome::getBpm() const
{
    return state_.bpm.load();
}


int Metronome::getBeatsPerBar() const
{
    return state_.beatsPerBar;
}

int Metronome::getSubdivisions() const
{
    return state_.subdivisions;
}

double Metronome::getVolume() const
{
    return state_.volume;
}

double Metronome::getClickFreqStrong() const
{
    return state_.clickFreqStrong;
}

double Metronome::getClickFreqWeak() const
{
    return state_.clickFreqWeak;
}

double Metronome::getClickDuration() const
{
    return state_.clickDurationSec;
}


void Metronome::handleSignal(int)
{
    globalRunning = false;
}



int Metronome::audioCallback(void* outputBuffer, void*, unsigned int nBufferFrames, double, RtAudioStreamStatus status, void* userData)
{
    State* state = static_cast<State*>(userData);
    float* out = static_cast<float*>(outputBuffer);

    if (status) {
        std::cerr << "Stream under/overrun detected." << std::endl;
    }

    const double twoPi = 6.28318530717958647692;
    int clickLenSamples = static_cast<int>(state->clickDurationSec * state->sampleRate);

    // Calculate note length in quarter notes
    double noteLength = 4.0 / state->timeSigDenominator; // e.g., 4/4=1, 8=0.5, 2=2
    // Calculate beat duration in seconds
    double beatDurationSec = (60.0 / state->bpm.load()) * noteLength;
    int samplesPerBeat = static_cast<int>(beatDurationSec * state->sampleRate);

    for (unsigned int i = 0; i < nBufferFrames; ++i) {
        if (state->samplesUntilBeat <= 0) {
            int samplesPerSub = samplesPerBeat / state->subdivisions;

            state->samplesUntilBeat = samplesPerSub;
            state->clickSamplesRemaining = clickLenSamples;

            // Advance subdivision index
            state->subdivisionIndex++;
            if (state->subdivisionIndex >= state->subdivisions) {
                state->subdivisionIndex = 0;
                state->beatIndex = (state->beatIndex + 1) % state->beatsPerBar;
                // If a callback is set, invoke it with the new beat index
                if (static_cast<Metronome*>(userData)->beatCallback) {
                    static_cast<Metronome*>(userData)->beatCallback(state->beatIndex);
                }
            }
        }

        float sample = 0.0f;

        if (state->clickSamplesRemaining > 0) {
            // Strong beat: first sub-beat of first beat
            bool strongBeat = (state->beatIndex == 0 && state->subdivisionIndex == 0);
            // Subdivision click: not strong, not first sub-beat
            bool subClick = (state->subdivisionIndex != 0);

            double freq;
            if (strongBeat) {
                freq = state->clickFreqStrong;
            } else if (subClick) {
                freq = state->clickFreqWeak;
            } else {
                freq = state->clickFreqWeak;
            }

            double env = 0.0;
            if (clickLenSamples > 0) {
                env = static_cast<double>(state->clickSamplesRemaining) / static_cast<double>(clickLenSamples);
            }

            double volume = state->volume;
            if (subClick) {
                volume *= 0.5; // make subdivisions quieter
            }

            sample = static_cast<float>(volume * env * std::sin(state->phase));

            state->phase += twoPi * freq / state->sampleRate;
            if (state->phase > twoPi) {
                state->phase -= twoPi;
            }

            state->clickSamplesRemaining = state->clickSamplesRemaining - 1;
        }

        out[i] = sample;
        state->samplesUntilBeat = state->samplesUntilBeat - 1;
    }

    if (globalRunning) {
        return 0;
    } else {
        return 1;
    }
}


bool Metronome::start()
{
    if (running_) {
        return false;
    }

    globalRunning = true;
    std::signal(SIGINT, handleSignal);

    if (dac_.getDeviceCount() < 1) {
        std::cerr << "No audio output device found." << std::endl;
        return false;
    }

    RtAudio::StreamParameters params;
    params.deviceId = dac_.getDefaultOutputDevice();
    params.nChannels = 1;
    params.firstChannel = 0;

    unsigned int bufferFrames = 256;
    unsigned int sampleRate = 48000;

    state_.sampleRate = static_cast<double>(sampleRate);
    state_.subdivisionIndex = state_.subdivisions - 1;
    state_.samplesUntilBeat = 1;
    state_.beatIndex = 0;
    state_.clickSamplesRemaining = 0;

    dac_.openStream(&params, nullptr, RTAUDIO_FLOAT32, sampleRate, &bufferFrames, &audioCallback, &state_);
    dac_.startStream();
    running_ = true;
    return true;
}


void Metronome::stop()
{
    globalRunning = false;

    if (dac_.isStreamRunning()) {
        dac_.stopStream();
    }

    if (dac_.isStreamOpen()) {
        dac_.closeStream();
    }

    running_ = false;
}


bool Metronome::isRunning() const
{
    return running_;
}


double Metronome::parseDoubleArg(int argc, char** argv, const char* flag, double defaultValue)
{
    for (int i = 1; i < argc - 1; ++i) {
        if (std::strcmp(argv[i], flag) == 0) {
            return std::atof(argv[i + 1]);
        }
    }
    return defaultValue;
}


int Metronome::parseIntArg(int argc, char** argv, const char* flag, int defaultValue)
{
    for (int i = 1; i < argc - 1; ++i) {
        if (std::strcmp(argv[i], flag) == 0) {
            return std::atoi(argv[i + 1]);
        }
    }
    return defaultValue;
}



void Metronome::printUsage(const char* progName)
{
    std::cout << "Usage: " << progName << " [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --bpm <value>            Set beats per minute (20-400)" << std::endl;
    std::cout << "  --beats <value>          Set beats per bar (default 4)" << std::endl;
    std::cout << "  --subdivisions <value>   Set subdivisions per beat (default 1)" << std::endl;
    std::cout << "  --volume <value>         Set click volume (0.0-1.0, default 0.7)" << std::endl;
    std::cout << "  --click-strong <freq>    Set strong beat click frequency (Hz)" << std::endl;
    std::cout << "  --click-weak <freq>      Set weak beat click frequency (Hz)" << std::endl;
    std::cout << "  --click-duration <sec>   Set click duration in seconds" << std::endl;
    std::cout << "  --help                   Show this help message" << std::endl;
}
