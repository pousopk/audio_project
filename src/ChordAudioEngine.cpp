#/**
# * @file ChordAudioEngine.cpp
# * @brief Implements the ChordAudioEngine class for real-time chord playback, effects, and synchronization.
# */
#include "../strumming/StrummingPattern.h"
#include <atomic>
#include "ChordAudioEngine.h"
#include "Metronome.h"
#include <cmath>
#include <cstring>
#include <map>
#include "../include/GuitarChordShape.h"
#include "../utils/GuitarChordShapes.h"
#include <QString>
#include <vector>
#include <array>
#include <iostream>
#include <complex>
#include "../utils/fft.h"

void ChordAudioEngine::resetChordSyncState() {
    measureSampleCounter_ = 0;
    chordMeasureCount_ = 0;
    started_ = false;
}

void ChordAudioEngine::setMetronome(Metronome* m) {
    metronome = m;
}

ChordAudioEngine::ChordAudioEngine() {
    fft_input_buffer_.resize(1024, 0.0f);
    spectrum_magnitudes_.resize(fft_input_buffer_.size() / 2, -100.0f);
}

ChordAudioEngine::~ChordAudioEngine() { stop(); }

void ChordAudioEngine::setProgression(const ChordProgression& prog) {
    progression = prog;
    currentChordIndex = 0;
    samplesLeftInChord = 0;
}

void ChordAudioEngine::setBpm(double b) { bpm = b; }
void ChordAudioEngine::setSampleRate(unsigned int rate) {
    sampleRate = rate;
    synth_.setSampleRate(rate);
    fxChain_.setSampleRate(static_cast<float>(rate));
}
bool ChordAudioEngine::isRunning() const { return running; }

void ChordAudioEngine::start() {
    if (running) return;
    RtAudio::StreamParameters params;
    params.deviceId = dac.getDefaultOutputDevice();
    params.nChannels = 1;
    params.firstChannel = 0;
    unsigned int bufferFrames = 256;
    running = true;
    // Reset sync state so playback always aligns with the next measure boundary
    resetChordSyncState();
    // --- Always wait for the next strong beat (start of a new bar) before playing the first chord ---
    if (metronome) {
        int beatsPerBar = metronome->getBeatsPerBar();
        int denominator = metronome->getTimeSignatureDenominator();
        double bpmToUse = metronome->getBpm();
        double noteLength = 4.0 / denominator;
        double secondsPerBeat = (60.0 / bpmToUse) * noteLength;
        int samplesPerBeat = static_cast<int>(secondsPerBeat * sampleRate);
        int beatIdx = metronome->getCurrentBeatIndex();
        // Wait until the next beat 0 (strong beat)
        // This logic was moved into the audio callback to handle sync more robustly,
        // but we can pre-calculate the initial silence here.
        measureSampleCounter_ = (beatIdx == 0)
                                  ? 0
                                  : samplesPerBeat * (beatsPerBar - beatIdx);
    }
    try {
        dac.openStream(&params, nullptr, RTAUDIO_FLOAT32, sampleRate, &bufferFrames, &audioCallback, this);
        dac.startStream();
    } catch (const std::exception& e) {
        std::cerr << "RtAudio error: " << e.what() << std::endl;
        running = false;
    }
}

void ChordAudioEngine::stop() {
    running = false;
    if (dac.isStreamRunning()) dac.stopStream();
    if (dac.isStreamOpen()) dac.closeStream();
    // Reset sync state so next start aligns with measure boundary
    resetChordSyncState();
}
// Helper to reset static sync state in fillBuffer

void ChordAudioEngine::setReverbMix(float mix) {
    fxChain_.setReverbMix(mix);
}

void ChordAudioEngine::setDelayTime(float time_ms) {
    fxChain_.setDelayTime(time_ms);
}

void ChordAudioEngine::setDelayFeedback(float feedback) {
    fxChain_.setDelayFeedback(feedback);
}

void ChordAudioEngine::setDelayMix(float mix) {
    fxChain_.setDelayMix(mix);
}

void ChordAudioEngine::setDistortionDrive(float drive) {
    fxChain_.setDistortionDrive(drive);
}

void ChordAudioEngine::setDistortionMix(float mix) {
    fxChain_.setDistortionMix(mix);
}

void ChordAudioEngine::setCompressorThreshold(float threshold_dB) {
    fxChain_.setCompressorThreshold(threshold_dB);
}

void ChordAudioEngine::setCompressorRatio(float ratio) {
    fxChain_.setCompressorRatio(ratio);
}

void ChordAudioEngine::setCompressorAttack(float attack_ms) {
    fxChain_.setCompressorAttack(attack_ms);
}

void ChordAudioEngine::setCompressorRelease(float release_ms) {
    fxChain_.setCompressorRelease(release_ms);
}

void ChordAudioEngine::setEQLowGain(float gain_db) {
    fxChain_.setEQLowGain(gain_db);
}

void ChordAudioEngine::setEQMidGain(float gain_db) {
    fxChain_.setEQMidGain(gain_db);
}

void ChordAudioEngine::setEQHighGain(float gain_db) {
    fxChain_.setEQHighGain(gain_db);
}

void ChordAudioEngine::setLimiterThreshold(float threshold_db) {
    fxChain_.setLimiterThreshold(threshold_db);
}

void ChordAudioEngine::setLimiterRelease(float release_ms) {
    fxChain_.setLimiterRelease(release_ms);
}

void ChordAudioEngine::setChorusRate(float rate_hz) {
    fxChain_.setChorusRate(rate_hz);
}

void ChordAudioEngine::setChorusDepth(float depth_ms) {
    fxChain_.setChorusDepth(depth_ms);
}

void ChordAudioEngine::setChorusMix(float mix) {
    fxChain_.setChorusMix(mix);
}

void ChordAudioEngine::setGateThreshold(float threshold_db) {
    fxChain_.setGateThreshold(threshold_db);
}

void ChordAudioEngine::setGateRelease(float release_ms) {
    fxChain_.setGateRelease(release_ms);
}

void ChordAudioEngine::setFlangerRate(float rate_hz) {
    fxChain_.setFlangerRate(rate_hz);
}

void ChordAudioEngine::setFlangerDepth(float depth_ms) {
    fxChain_.setFlangerDepth(depth_ms);
}

void ChordAudioEngine::setFlangerFeedback(float feedback) {
    fxChain_.setFlangerFeedback(feedback);
}

void ChordAudioEngine::setFlangerMix(float mix) {
    fxChain_.setFlangerMix(mix);
}

void ChordAudioEngine::setPhaserRate(float rate_hz) {
    fxChain_.setPhaserRate(rate_hz);
}

void ChordAudioEngine::setPhaserDepth(float depth) {
    fxChain_.setPhaserDepth(depth);
}

void ChordAudioEngine::setPhaserFeedback(float feedback) {
    fxChain_.setPhaserFeedback(feedback);
}

void ChordAudioEngine::setPhaserMix(float mix) {
    fxChain_.setPhaserMix(mix);
}

void ChordAudioEngine::setTremoloRate(float rate_hz) {
    fxChain_.setTremoloRate(rate_hz);
}

void ChordAudioEngine::setTremoloDepth(float depth) {
    fxChain_.setTremoloDepth(depth);
}

float ChordAudioEngine::getCompressorGainReductionDB() const {
    return fxChain_.getCompressorGainReductionDB();
}

std::vector<float> ChordAudioEngine::getSpectrumData() const {
    std::lock_guard<std::mutex> lock(spectrum_mutex_);
    // Return a copy of the data for thread safety
    return spectrum_magnitudes_;
}

void ChordAudioEngine::setFXOrder(const std::vector<std::string>& newOrder) {
    fxChain_.setOrder(newOrder);
}


int ChordAudioEngine::audioCallback(void* outputBuffer, void*, unsigned int nBufferFrames, double, RtAudioStreamStatus status, void* userData) {
    auto* engine = static_cast<ChordAudioEngine*>(userData);
    float* out = static_cast<float*>(outputBuffer);
    if (status) std::cerr << "Stream under/overrun detected." << std::endl;
    engine->fillBuffer(out, nBufferFrames);
    return engine->running ? 0 : 1;
}

void ChordAudioEngine::fillBuffer(float* out, unsigned int nFrames) {
    // Prepare chord audio
    const auto& chords = progression.getChords();
    if (chords.empty()) {
        for (unsigned int i = 0; i < nFrames; ++i) out[i] = 0.0f;
        return;
    }

    synth_.setVolume(chordVolume);

    // --- Synchronize chord changes to measure boundaries ---
    // Always use metronome's BPM, beats per bar, and denominator for timing
    int beatsPerBar = 4;
    int denominator = 4;
    double bpmToUse = bpm;
    if (metronome) {
        beatsPerBar = metronome->getBeatsPerBar();
        denominator = metronome->getTimeSignatureDenominator();
        bpmToUse = metronome->getBpm();
    }
    double noteLength = 4.0 / denominator;
    double secondsPerBeat = (60.0 / bpmToUse) * noteLength;
    int samplesPerBeat = static_cast<int>(secondsPerBeat * sampleRate);
    int samplesPerMeasure = samplesPerBeat * beatsPerBar;
    unsigned int frame = 0;

    while (frame < nFrames) {
        // Wait until measureSampleCounter == 0 (the very first sample of a measure) to start playback
        if (!started_) {
            if (measureSampleCounter_ == 0) {
                started_ = true;
            } else {
                int silenceFrames = std::min(static_cast<int>(nFrames - frame), samplesPerMeasure - measureSampleCounter_);
                for (int f = 0; f < silenceFrames; ++f) {
                    out[frame + f] = 0.0f;
                }
                frame += silenceFrames;
                measureSampleCounter_ += silenceFrames;
                if (measureSampleCounter_ >= samplesPerMeasure) {
                    measureSampleCounter_ = 0;
                }
                continue;
            }
        }
        if (currentChordIndex >= static_cast<int>(chords.size())) {
            currentChordIndex = 0; // loop back to first chord
            lastChordIndex_ = -1;   // force callback for first chord of new loop
        }
        const ChordChange& chord = chords[currentChordIndex];
        // --- Chord change event ---
        if (lastChordIndex_ != currentChordIndex && chordChangeCallback) {
            chordChangeCallback(chord.chordName.toStdString());
            lastChordIndex_ = currentChordIndex;
        }
        if (chordMeasureCount_ <= 0) {
            chordMeasureCount_ = chord.bars;
        }
        int samplesLeftInMeasure = samplesPerMeasure - measureSampleCounter_;
        int framesThisMeasure = std::min(static_cast<int>(nFrames - frame), samplesLeftInMeasure);
        // --- Use lowest available guitar inversion for this chord ---
        // Use the same logic as generateChordInversions in main.cpp
        std::vector<int> shapeFrets;
        auto it = kGuitarChordShapes.find(chord.chordName.toStdString());
        if (it != kGuitarChordShapes.end()) {
            shapeFrets = it->second;
        } else {
            // fallback: all muted
            shapeFrets = {-1, -1, -1, -1, -1, -1};
        }

        // Use the strumming pattern for this chord
        int strumIdx = chord.strummingPatternIndex;
        StrummingPattern currentStrumPattern = (strumIdx >= 0 && strumIdx < (int)kStrummingPatterns.size()) ? kStrummingPatterns[strumIdx] : strummingPattern;

        synth_.process(out + frame, framesThisMeasure, shapeFrets, currentStrumPattern, samplesPerMeasure, measureSampleCounter_, beatsPerBar, samplesPerBeat);

        // Apply effects to the newly synthesized audio frame
        fxChain_.process(out + frame, framesThisMeasure);

        frame += framesThisMeasure;
        measureSampleCounter_ += framesThisMeasure;
        if (measureSampleCounter_ >= samplesPerMeasure) {
            measureSampleCounter_ = 0;
            chordMeasureCount_--;
            if (chordMeasureCount_ <= 0) {
                currentChordIndex++;
            }
        }
    }
    // Mix in metronome if present
    if (metronome) {
        std::vector<float> metroBuf(nFrames, 0.0f);
        metronome->fillBuffer(metroBuf.data(), nFrames);
        for (unsigned int i = 0; i < nFrames; ++i) {
            out[i] += metroBuf[i];
        }
    }

    // --- Spectrum Analyzer Processing ---
    for (unsigned int i = 0; i < nFrames; ++i) {
        if (fft_input_pos_ < fft_input_buffer_.size()) {
            // Apply a window function (Hann) for better FFT results
            float window = 0.5f * (1.0f - cosf(2.0f * FFT::PI * fft_input_pos_ / (fft_input_buffer_.size() - 1)));
            fft_input_buffer_[fft_input_pos_++] = out[i] * window;
        }

        if (fft_input_pos_ >= fft_input_buffer_.size()) {
            // Buffer is full, perform FFT
            std::vector<std::complex<float>> fft_complex(fft_input_buffer_.size());
            for (size_t j = 0; j < fft_input_buffer_.size(); ++j) {
                fft_complex[j] = {fft_input_buffer_[j], 0.0f};
            }
            FFT::transform(fft_complex);

            // Calculate magnitudes in dB and update the shared buffer
            std::lock_guard<std::mutex> lock(spectrum_mutex_);
            for (size_t j = 0; j < spectrum_magnitudes_.size(); ++j) {
                float mag = std::abs(fft_complex[j]);
                spectrum_magnitudes_[j] = 20.0f * log10(mag + 1e-9); // Add epsilon to avoid log(0)
            }
            fft_input_pos_ = 0; // Reset for next block
        }
    }
}
