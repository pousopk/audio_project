#include "effects/Tremolo.h"
#include <cmath>

namespace {
constexpr float kPi = 3.14159265358979323846f;
}

Tremolo::Tremolo() {
    setSampleRate(sample_rate_);
}

void Tremolo::setSampleRate(float rate) {
    sample_rate_ = rate > 0 ? rate : 48000.0f;
    const float splitHz = 900.0f;
    split_alpha_ = 1.0f - std::exp(-2.0f * kPi * splitHz / sample_rate_);
}

void Tremolo::setRate(float rate_hz) {
    rate_hz_ = std::fmax(0.05f, rate_hz);
}

void Tremolo::setDepth(float depth) {
    depth_ = std::fmax(0.0f, std::fmin(1.0f, depth));
}

float Tremolo::process(float input) {
    // Drifted modulation engine around the main LFO.
    const float lfo_increment = 2.0f * kPi * rate_hz_ / sample_rate_;
    const float drift_increment = 2.0f * kPi * drift_rate_hz_ / sample_rate_;
    lfo_phase_ += lfo_increment;
    drift_phase_ += drift_increment;
    if (lfo_phase_ >= 2.0f * kPi) lfo_phase_ -= 2.0f * kPi;
    if (drift_phase_ >= 2.0f * kPi) drift_phase_ -= 2.0f * kPi;

    const float drift = drift_amount_ * std::sin(drift_phase_);
    const float lfoL = std::sin(lfo_phase_ + drift);
    const float lfoR = std::sin(lfo_phase_ + stereo_phase_offset_rad_ - drift);

    // Stereo movement matrix folded mono-safely.
    const float modLR = 0.5f * (lfoL + lfoR);
    const float psychoShaping = 0.5f + 0.5f * modLR;

    float dryLow = input;
    float dryHigh = 0.0f;
    if (band_split_enabled_) {
        split_state_ += split_alpha_ * (input - split_state_);
        dryLow = split_state_;
        dryHigh = input - split_state_;
    }

    // Psychoacoustic gain modulation: lows move less, highs move more.
    const float lowDepth = 0.6f * depth_;
    const float highDepth = 1.0f * depth_;
    const float lowGain = 1.0f - lowDepth * psychoShaping;
    const float highGain = 1.0f - highDepth * psychoShaping;
    float wet = band_split_enabled_ ? (dryLow * lowGain + dryHigh * highGain)
                                    : (input * (1.0f - depth_ * psychoShaping));

    if (saturation_enabled_) {
        wet = std::tanh(saturation_drive_ * wet) / std::tanh(saturation_drive_);
    }

    return input * (1.0f - mix_) + wet * mix_;
}