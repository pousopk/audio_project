#/**
# * @file Gate.cpp
# * @brief Implements the Gate audio effect for noise gating and signal muting.
# */
#include "effects/Gate.h"
#include <cmath>

namespace {
constexpr float kPi = 3.14159265358979323846f;
}

Gate::Gate() {
    setThreshold(threshold_db_);
    updateCoefficients();
}

void Gate::setSampleRate(float rate) {
    sample_rate_ = rate > 0 ? rate : 48000.0f;
    updateCoefficients();
}

float Gate::msToCoeff(float ms) const {
    const float clampedMs = std::max(0.1f, ms);
    return std::exp(-1.0f / (clampedMs * sample_rate_ / 1000.0f));
}

void Gate::updateCoefficients() {
    attack_coeff_ = msToCoeff(attack_ms_);
    release_coeff_ = msToCoeff(release_ms_);

    detector_attack_coeff_ = msToCoeff(2.0f);
    detector_release_coeff_ = msToCoeff(90.0f);
    rms_coeff_ = msToCoeff(35.0f);

    hold_samples_ = static_cast<int>(std::max(1.0f, hold_ms_ * sample_rate_ / 1000.0f));

    // 1-pole HPF coeff for sidechain detector.
    const float hz = std::clamp(sidechain_hpf_hz_, 80.0f, 150.0f);
    hpf_a_ = std::exp(-2.0f * kPi * hz / sample_rate_);

    // Low-pass for pseudo-side estimate used in mono-safe stereo linking.
    const float linkHz = 1200.0f;
    link_lp_alpha_ = 1.0f - std::exp(-2.0f * kPi * linkHz / sample_rate_);
}

void Gate::setThreshold(float threshold_dB) {
    threshold_db_ = std::clamp(threshold_dB, -80.0f, 6.0f);
    threshold_ = std::pow(10.0f, threshold_db_ / 20.0f);
}

void Gate::setRelease(float release_ms) {
    release_ms_ = std::clamp(release_ms, 20.0f, 800.0f);
    updateCoefficients();
}

float Gate::process(float input) {
    // Sidechain HPF.
    const float sc = hpf_a_ * (hpf_y1_ + input - hpf_x1_);
    hpf_x1_ = input;
    hpf_y1_ = sc;

    // Envelope detector: RMS + peak blend.
    const float scAbs = std::fabs(sc);
    if (scAbs > peak_env_) {
        peak_env_ = detector_attack_coeff_ * peak_env_ + (1.0f - detector_attack_coeff_) * scAbs;
    } else {
        peak_env_ = detector_release_coeff_ * peak_env_ + (1.0f - detector_release_coeff_) * scAbs;
    }

    const float scSq = sc * sc;
    rms_env_sq_ = rms_coeff_ * rms_env_sq_ + (1.0f - rms_coeff_) * scSq;
    const float rmsEnv = std::sqrt(std::max(0.0f, rms_env_sq_));
    const float detector = detector_blend_ * peak_env_ + (1.0f - detector_blend_) * rmsEnv;

    // Mono-safe stereo linking approximation: blend with a side-like estimate and link by max.
    link_lp_state_ += link_lp_alpha_ * (input - link_lp_state_);
    const float sideLike = std::fabs(input - link_lp_state_);
    const float linkedDetector = std::max(detector, 0.7f * sideLike);

    // Hysteresis decision logic with hold.
    const float openThreshold = threshold_;
    const float closeThreshold = threshold_ * std::pow(10.0f, -std::clamp(hysteresis_db_, 3.0f, 10.0f) / 20.0f);

    if (gate_open_) {
        if (linkedDetector >= closeThreshold) {
            hold_counter_samples_ = hold_samples_;
        } else if (hold_counter_samples_ > 0) {
            --hold_counter_samples_;
        } else {
            gate_open_ = false;
        }
    } else if (linkedDetector >= openThreshold) {
        gate_open_ = true;
        hold_counter_samples_ = hold_samples_;
    }

    // Soft gain curve around threshold with knee.
    const float detectorDb = 20.0f * std::log10(linkedDetector + 1.0e-9f);
    const float knee = std::clamp(knee_db_, 1.0f, 12.0f);
    const float kneeStart = threshold_db_ - 0.5f * knee;
    const float kneeEnd = threshold_db_ + 0.5f * knee;
    float soft = (detectorDb - kneeStart) / std::max(0.1f, kneeEnd - kneeStart);
    soft = std::clamp(soft, 0.0f, 1.0f);
    soft = soft * soft * (3.0f - 2.0f * soft); // Smoothstep

    const float closedFloor = 0.01f;
    const float targetGain = gate_open_ ? 1.0f : (closedFloor + 0.2f * soft);

    // Attack / release gain envelope.
    if (targetGain > gain_env_) {
        gain_env_ = attack_coeff_ * gain_env_ + (1.0f - attack_coeff_) * targetGain;
    } else {
        gain_env_ = release_coeff_ * gain_env_ + (1.0f - release_coeff_) * targetGain;
    }

    return input * gain_env_;
}