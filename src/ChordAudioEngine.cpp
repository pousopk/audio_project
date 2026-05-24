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
#include <vector>
#include <array>
#include <iostream>
#include <complex>
#include <limits>
#include "../utils/fft.h"

namespace {
constexpr std::array<int, 6> kStringMidi = {40, 45, 50, 55, 59, 64};

struct KarplusPresetValues {
    double decay;
    double brightness;
    double transient;
    double dispersion;
    double drive;
    double body;
    double sympathetic;
};

KarplusPresetValues getKarplusPresetValues(ChordAudioEngine::KarplusPreset preset) {
    switch (preset) {
        case ChordAudioEngine::KarplusPreset::Warm:
            return {0.994, 0.10, 0.12, 0.04, 1.1, 0.75, 0.05};
        case ChordAudioEngine::KarplusPreset::Bright:
            return {0.989, 0.26, 0.35, 0.14, 2.0, 0.40, 0.02};
        case ChordAudioEngine::KarplusPreset::Muted:
            return {0.978, 0.08, 0.06, 0.02, 0.9, 0.35, 0.00};
        case ChordAudioEngine::KarplusPreset::Custom:
        case ChordAudioEngine::KarplusPreset::Balanced:
        default:
            return {0.992, 0.14, 0.20, 0.08, 1.3, 0.55, 0.03};
    }
}
} // namespace

void ChordAudioEngine::resetChordSyncState() {
    measureSampleCounter_ = 0;
    chordMeasureCount_ = 0;
    started_ = false;
    activeVoicedChordIndex_ = -1;
    activeVoicedShape_.clear();
    previousVoicedShape_.clear();
}

std::vector<int> ChordAudioEngine::applyVoiceLeading(const std::vector<int>& baseShape) const {
    if (baseShape.empty() || previousVoicedShape_.empty()) {
        return baseShape;
    }

    std::vector<int> previousMidis;
    for (int s = 0; s < static_cast<int>(previousVoicedShape_.size()) && s < 6; ++s) {
        if (previousVoicedShape_[s] >= 0) {
            previousMidis.push_back(kStringMidi[s] + previousVoicedShape_[s]);
        }
    }
    if (previousMidis.empty()) {
        return baseShape;
    }

    std::vector<int> voiced = baseShape;
    for (int s = 0; s < static_cast<int>(voiced.size()) && s < 6; ++s) {
        const int fret = voiced[s];
        if (fret < 0) continue;

        int targetMidi = previousMidis.front();
        int bestTargetDist = std::numeric_limits<int>::max();
        if (s < static_cast<int>(previousVoicedShape_.size()) && previousVoicedShape_[s] >= 0) {
            targetMidi = kStringMidi[s] + previousVoicedShape_[s];
        } else {
            for (int pm : previousMidis) {
                const int d = std::abs(pm - (kStringMidi[s] + fret));
                if (d < bestTargetDist) {
                    bestTargetDist = d;
                    targetMidi = pm;
                }
            }
        }

        int bestFret = fret;
        int bestCost = std::numeric_limits<int>::max();
        const std::array<int, 3> candidates = {fret - 12, fret, fret + 12};
        for (int cand : candidates) {
            if (cand < 0 || cand > 24) continue;
            const int candMidi = kStringMidi[s] + cand;
            int cost = std::abs(candMidi - targetMidi);
            if (cand != fret) cost += 2;
            if (cost < bestCost) {
                bestCost = cost;
                bestFret = cand;
            }
        }
        voiced[s] = bestFret;
    }

    return voiced;
}

void ChordAudioEngine::setMetronome(Metronome* m) {
    metronome = m;
}

ChordAudioEngine::ChordAudioEngine() {
    fft_input_buffer_.resize(1024, 0.0f);
    spectrum_magnitudes_.resize(fft_input_buffer_.size() / 2, -100.0f);
}

void ChordAudioEngine::setSynthType(SynthType type) {
    synthType_ = type;
}

void ChordAudioEngine::setGuitarFilterCoefficient(double coeff) {
    synth_.setFilterCoefficient(coeff);
}

void ChordAudioEngine::setGuitarDecayFactor(double factor) {
    synth_.setDecayFactor(factor);
}

void ChordAudioEngine::setSubtractiveWaveformSaw(bool saw) {
    subtractiveSynth_.setWaveformSaw(saw);
}

void ChordAudioEngine::setSubtractiveCutoff(double cutoffHz) {
    subtractiveSynth_.setCutoff(cutoffHz);
}

void ChordAudioEngine::setSubtractiveEnvelope(double attack, double decay, double sustain, double release) {
    subtractiveSynth_.setEnvelope(attack, decay, sustain, release);
}

void ChordAudioEngine::setFMModRatio(double ratio) {
    fmSynth_.setModRatio(ratio);
}

void ChordAudioEngine::setFMModIndex(double index) {
    fmSynth_.setModIndex(index);
}

void ChordAudioEngine::setFMEnvelope(double attack, double decay, double sustain, double release) {
    fmSynth_.setEnvelope(attack, decay, sustain, release);
}

void ChordAudioEngine::setWavetableEnvelope(double attack, double decay, double sustain, double release) {
    wavetableSynth_.setEnvelope(attack, decay, sustain, release);
}

void ChordAudioEngine::setKarplusCustomDecay(double decay) {
    karplusPreset_ = KarplusPreset::Custom;
    karplusStrongSynth_.setDecay(decay);
}

void ChordAudioEngine::setKarplusExciterBrightness(double brightness) {
    karplusPreset_ = KarplusPreset::Custom;
    karplusStrongSynth_.setExciterBrightness(brightness);
}

void ChordAudioEngine::setKarplusPickTransient(double amount) {
    karplusPreset_ = KarplusPreset::Custom;
    karplusStrongSynth_.setPickTransient(amount);
}

void ChordAudioEngine::setKarplusDispersion(double amount) {
    karplusPreset_ = KarplusPreset::Custom;
    karplusStrongSynth_.setDispersion(amount);
}

void ChordAudioEngine::setKarplusDrive(double drive) {
    karplusPreset_ = KarplusPreset::Custom;
    karplusStrongSynth_.setDrive(drive);
}

void ChordAudioEngine::setKarplusBodyResonance(double amount) {
    karplusPreset_ = KarplusPreset::Custom;
    karplusStrongSynth_.setBodyResonance(amount);
}

void ChordAudioEngine::setKarplusSympatheticResonance(double amount) {
    karplusPreset_ = KarplusPreset::Custom;
    karplusStrongSynth_.setSympatheticResonance(amount);
}

void ChordAudioEngine::applyKarplusPreset(KarplusPreset preset) {
    if (preset == KarplusPreset::Custom) {
        karplusPreset_ = KarplusPreset::Custom;
        return;
    }

    const KarplusPresetValues p = getKarplusPresetValues(preset);
    karplusStrongSynth_.setDecay(p.decay);
    karplusStrongSynth_.setExciterBrightness(p.brightness);
    karplusStrongSynth_.setPickTransient(p.transient);
    karplusStrongSynth_.setDispersion(p.dispersion);
    karplusStrongSynth_.setDrive(p.drive);
    karplusStrongSynth_.setBodyResonance(p.body);
    karplusStrongSynth_.setSympatheticResonance(p.sympathetic);
    karplusPreset_ = preset;
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
    subtractiveSynth_.setSampleRate(rate);
    fmSynth_.setSampleRate(rate);
    wavetableSynth_.setSampleRate(rate);
    karplusStrongSynth_.setSampleRate(rate);
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

void ChordAudioEngine::setCompressorKnee(float knee_db) {
    fxChain_.setCompressorKnee(knee_db);
}

void ChordAudioEngine::setCompressorSaturationDrive(float drive) {
    fxChain_.setCompressorSaturationDrive(drive);
}

void ChordAudioEngine::setCompressorDetectorBlend(float blend) {
    fxChain_.setCompressorDetectorBlend(blend);
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

void ChordAudioEngine::setEQSaturationEnabled(bool enabled) {
    fxChain_.setEQSaturationEnabled(enabled);
}

void ChordAudioEngine::setEQSaturationDrive(float drive) {
    fxChain_.setEQSaturationDrive(drive);
}

void ChordAudioEngine::setEQMSAmount(float amount) {
    fxChain_.setEQMSAmount(amount);
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

void ChordAudioEngine::setFXEnabled(bool enabled) {
    fxChain_.setEnabled(enabled);
}

bool ChordAudioEngine::isFXEnabled() const {
    return fxChain_.isEnabled();
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
    subtractiveSynth_.setVolume(chordVolume);
    fmSynth_.setVolume(chordVolume);
    wavetableSynth_.setVolume(chordVolume);
    karplusStrongSynth_.setVolume(chordVolume);

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
            chordChangeCallback(chord.chordName);
            lastChordIndex_ = currentChordIndex;
        }
        if (chordMeasureCount_ <= 0) {
            chordMeasureCount_ = chord.bars;
        }
        int samplesLeftInMeasure = samplesPerMeasure - measureSampleCounter_;
        int framesThisMeasure = std::min(static_cast<int>(nFrames - frame), samplesLeftInMeasure);
        std::vector<int> shapeFrets = activeVoicedShape_;
        if (activeVoicedChordIndex_ != currentChordIndex || shapeFrets.empty()) {
            std::vector<int> baseShape;
            auto it = kGuitarChordShapes.find(chord.chordName);
            if (it != kGuitarChordShapes.end()) {
                baseShape = it->second;
            } else {
                // fallback: all muted
                baseShape = {-1, -1, -1, -1, -1, -1};
            }
            shapeFrets = applyVoiceLeading(baseShape);
            activeVoicedShape_ = shapeFrets;
            activeVoicedChordIndex_ = currentChordIndex;
            previousVoicedShape_ = shapeFrets;
        }

        // Use the strumming pattern for this chord
        int strumIdx = chord.strummingPatternIndex;
        StrummingPattern currentStrumPattern = (strumIdx >= 0 && strumIdx < (int)kStrummingPatterns.size()) ? kStrummingPatterns[strumIdx] : strummingPattern;

        switch (synthType_) {
            case SynthType::Guitar:
                synth_.process(out + frame, framesThisMeasure, shapeFrets, currentStrumPattern, samplesPerMeasure, measureSampleCounter_, beatsPerBar, samplesPerBeat);
                break;
            case SynthType::Subtractive:
                subtractiveSynth_.process(out + frame, framesThisMeasure, shapeFrets, currentStrumPattern, samplesPerMeasure, measureSampleCounter_, beatsPerBar, samplesPerBeat);
                break;
            case SynthType::FM:
                fmSynth_.process(out + frame, framesThisMeasure, shapeFrets, currentStrumPattern, samplesPerMeasure, measureSampleCounter_, beatsPerBar, samplesPerBeat);
                break;
            case SynthType::Wavetable:
                wavetableSynth_.process(out + frame, framesThisMeasure, shapeFrets, currentStrumPattern, samplesPerMeasure, measureSampleCounter_, beatsPerBar, samplesPerBeat);
                break;
            case SynthType::KarplusStrong:
                karplusStrongSynth_.process(out + frame, framesThisMeasure, shapeFrets, currentStrumPattern, samplesPerMeasure, measureSampleCounter_, beatsPerBar, samplesPerBeat);
                break;
        }

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
