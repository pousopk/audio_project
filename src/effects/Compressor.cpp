#/**
# * @file Compressor.cpp
# * @brief Implements the Compressor audio effect for dynamic range control.
# */
#include "effects/Compressor.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr float kEps = 1.0e-9f;
constexpr float kPi = 3.14159265358979323846f;
}

Compressor::Compressor() {
    updateCoefficients();
}

void Compressor::setSampleRate(float rate) {
    sample_rate_ = rate > 0 ? rate : 48000.0f;
    updateCoefficients();
}

float Compressor::timeMsToCoeff(float ms, float sr) const {
    const float clampedMs = std::max(0.05f, ms);
    const float samples = clampedMs * 0.001f * std::max(1.0f, sr);
    return std::exp(-1.0f / samples);
}

float Compressor::linearToDb(float x) const {
    return 20.0f * std::log10(std::max(kEps, x));
}

float Compressor::dbToLinear(float db) const {
    return std::pow(10.0f, db / 20.0f);
}

float Compressor::softKneeGainReductionDb(float detectorDb) const {
    const float over = detectorDb - threshold_db_;
    const float halfKnee = 0.5f * knee_db_;
    const float slope = 1.0f - 1.0f / std::max(1.0f, ratio_);

    if (knee_db_ <= 0.0f) {
        return (over > 0.0f) ? -(slope * over) : 0.0f;
    }
    if (over <= -halfKnee) {
        return 0.0f;
    }
    if (over >= halfKnee) {
        return -(slope * over);
    }

    const float x = over + halfKnee;
    const float compressed = slope * (x * x) / (2.0f * knee_db_);
    return -compressed;
}

void Compressor::updateCoefficients() {
    const float detSr = sample_rate_ * static_cast<float>(kOversampling);

    // High-pass coefficient for sidechain pre-emphasis.
    const float rc = 1.0f / (2.0f * kPi * std::max(20.0f, sidechain_hp_hz_));
    const float dt = 1.0f / std::max(1.0f, detSr);
    hp_alpha_ = rc / (rc + dt);

    // Gain reduction smoothing around ~5ms.
    gr_smooth_coeff_ = timeMsToCoeff(5.0f, sample_rate_);
}

void Compressor::setThreshold(float threshold_dB) {
    threshold_db_ = std::clamp(threshold_dB, -60.0f, 0.0f);
}

void Compressor::setRatio(float ratio) {
    ratio_ = std::clamp(ratio, 1.0f, 20.0f);
}

void Compressor::setAttack(float attack_ms) {
    attack_ms_ = std::clamp(attack_ms, 0.5f, 200.0f);
}

void Compressor::setRelease(float release_ms) {
    release_ms_ = std::clamp(release_ms, 10.0f, 1000.0f);
}

void Compressor::setKnee(float knee_db) {
    knee_db_ = std::clamp(knee_db, 0.0f, 24.0f);
}

void Compressor::setSaturationDrive(float drive) {
    saturation_drive_ = std::clamp(drive, 0.5f, 4.0f);
}

void Compressor::setDetectorBlend(float blend) {
    hybrid_blend_ = std::clamp(blend, 0.0f, 1.0f);
}

float Compressor::process(float input) {
    // Oversampling path: midpoint + current sample to improve sidechain behavior.
    const float subSamples[kOversampling] = {
        0.5f * (prev_input_ + input),
        input
    };
    prev_input_ = input;

    const float detSr = sample_rate_ * static_cast<float>(kOversampling);
    for (int i = 0; i < kOversampling; ++i) {
        const float x = subSamples[i];

        // Sidechain filter (high-pass).
        const float sc = hp_alpha_ * (hp_y1_ + x - hp_x1_);
        hp_x1_ = x;
        hp_y1_ = sc;
        const float level = std::fabs(sc);

        // Hybrid peak/RMS detector.
        const float peakAttackCoeff = timeMsToCoeff(attack_ms_ * 0.35f, detSr);
        const float peakReleaseCoeff = timeMsToCoeff(release_ms_ * 0.6f, detSr);
        const float peakCoeff = (level > peak_env_) ? peakAttackCoeff : peakReleaseCoeff;
        peak_env_ = peakCoeff * peak_env_ + (1.0f - peakCoeff) * level;

        const float sq = level * level;
        const float rmsAttackCoeff = timeMsToCoeff(attack_ms_ * 0.9f, detSr);
        const float rmsReleaseCoeff = timeMsToCoeff(release_ms_ * 1.2f, detSr);
        const float rmsCoeff = (sq > rms_state_) ? rmsAttackCoeff : rmsReleaseCoeff;
        rms_state_ = rmsCoeff * rms_state_ + (1.0f - rmsCoeff) * sq;
        const float rms = std::sqrt(std::max(0.0f, rms_state_));

        const float detector = hybrid_blend_ * peak_env_ + (1.0f - hybrid_blend_) * rms;
        const float detectorDb = linearToDb(detector);

        // Program-dependent timing envelope.
        const float overDb = std::max(0.0f, detectorDb - threshold_db_);
        const float attMul = (overDb > 12.0f) ? 0.45f : (overDb > 6.0f ? 0.65f : 1.0f);
        const float relMul = (overDb > 12.0f) ? 1.9f : (overDb > 6.0f ? 1.45f : 1.0f);
        const float attackCoeff = timeMsToCoeff(attack_ms_ * attMul, detSr);
        const float releaseCoeff = timeMsToCoeff(release_ms_ * relMul, detSr);
        const float envCoeff = (detector > detector_env_) ? attackCoeff : releaseCoeff;
        detector_env_ = envCoeff * detector_env_ + (1.0f - envCoeff) * detector;
    }

    // Soft-knee gain computer.
    const float envDb = linearToDb(detector_env_);
    const float targetGrDb = softKneeGainReductionDb(envDb);

    // Smoothed gain reduction.
    gr_smooth_db_ = gr_smooth_coeff_ * gr_smooth_db_ + (1.0f - gr_smooth_coeff_) * targetGrDb;

    // Stereo link logic (mono-safe placeholder for future stereo extension).
    linked_gr_db_ = stereo_link_ * gr_smooth_db_ + (1.0f - stereo_link_) * linked_gr_db_;

    float out = input * dbToLinear(linked_gr_db_);

    // Optional post-compression saturation.
    if (saturation_enabled_) {
        const float norm = std::tanh(saturation_drive_);
        out = std::tanh(saturation_drive_ * out) / std::max(1e-6f, norm);
    }

    gain_reduction_db_.store(std::min(0.0f, linked_gr_db_));

    return out;
}

float Compressor::getGainReductionDB() const {
    return gain_reduction_db_.load();
}