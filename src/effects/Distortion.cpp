#include "effects/Distortion.h"
#include <cmath>

namespace {
constexpr float kPi = 3.14159265358979323846f;
}

void Distortion::setSampleRate(float rate) {
    sample_rate_ = (rate > 0.0f) ? rate : 48000.0f;
    updateCoefficients();
}

void Distortion::updateCoefficients() {
    // Pre-filter stage: trim sub rumble and overly bright fizz before drive.
    const float hpCut = 95.0f;
    const float lpCut = 6200.0f;
    pre_hp_alpha_ = 1.0f - std::exp(-2.0f * kPi * hpCut / sample_rate_);
    pre_lp_alpha_ = 1.0f - std::exp(-2.0f * kPi * lpCut / sample_rate_);

    // Oversampling cleanup and tone stage.
    os_cleanup_alpha_ = 1.0f - std::exp(-2.0f * kPi * 9500.0f / sample_rate_);
    tone_lp_alpha_ = 1.0f - std::exp(-2.0f * kPi * 4200.0f / sample_rate_);

    // Multiband split points.
    mb_low_alpha_ = 1.0f - std::exp(-2.0f * kPi * 250.0f / sample_rate_);
    mb_mid_alpha_ = 1.0f - std::exp(-2.0f * kPi * 2200.0f / sample_rate_);

    // Side estimation for mono-safe stereo approximation.
    side_lp_alpha_ = 1.0f - std::exp(-2.0f * kPi * 1400.0f / sample_rate_);
}

float Distortion::shapedCore(float x) const {
    const float driveGain = 1.0f + 30.0f * drive_;
    const float bias = 0.18f * drive_;
    const float in = x * driveGain + bias;

    // Asymmetric shaping for more tube-like even/odd harmonic balance.
    const float pos = std::tanh(in);
    const float neg = std::tanh(0.75f * in);
    return (in >= 0.0f) ? pos : neg;
}

void Distortion::setDrive(float drive) {
    drive_ = std::clamp(drive, 0.0f, 1.0f);
}

void Distortion::setMix(float mix) {
    mix_ = std::clamp(mix, 0.0f, 1.0f);
}

float Distortion::process(float input) {
    if (sample_rate_ <= 0.0f) {
        setSampleRate(48000.0f);
    }

    // 1) Pre-filter (HP/LP shaping).
    pre_hp_lp_state_ += pre_hp_alpha_ * (input - pre_hp_lp_state_);
    const float hp = input - pre_hp_lp_state_;
    pre_lp_state_ += pre_lp_alpha_ * (hp - pre_lp_state_);
    const float pre = pre_lp_state_;

    // 2-4) Drive+bias -> waveshaper -> oversampling cleanup.
    float nonlinear = 0.0f;
    if (oversampling_enabled_) {
        const float x0 = 0.5f * (os_prev_input_ + pre);
        const float x1 = pre;
        const float y0 = shapedCore(x0);
        const float y1 = shapedCore(x1);
        os_prev_input_ = pre;
        nonlinear = 0.5f * (y0 + y1);
    } else {
        nonlinear = shapedCore(pre);
    }

    os_cleanup_lp_state_ += os_cleanup_alpha_ * (nonlinear - os_cleanup_lp_state_);
    float wet = os_cleanup_lp_state_;

    // 5) Tone filtering: brighter at low drive, darker at high drive.
    const float toneBlend = 0.25f + 0.6f * (1.0f - drive_);
    tone_lp_state_ += tone_lp_alpha_ * (wet - tone_lp_state_);
    wet = toneBlend * tone_lp_state_ + (1.0f - toneBlend) * wet;

    // 6) Optional multiband split and weighted recombination.
    if (multiband_enabled_) {
        mb_low_state_ += mb_low_alpha_ * (wet - mb_low_state_);
        const float low = mb_low_state_;
        mb_mid_lp_state_ += mb_mid_alpha_ * (wet - mb_mid_lp_state_);
        const float high = wet - mb_mid_lp_state_;
        const float mid = wet - low - high;

        const float lowDrive = 0.95f - 0.20f * drive_;
        const float midDrive = 1.0f + 0.15f * drive_;
        const float highDrive = 0.80f - 0.25f * drive_;
        wet = lowDrive * low + midDrive * mid + highDrive * high;
    }

    // 7) Stereo processing (mono-safe approximation).
    side_lp_state_ += side_lp_alpha_ * (wet - side_lp_state_);
    const float side = wet - side_lp_state_;
    const float left = wet + stereo_width_ * side;
    const float right = wet - stereo_width_ * side;
    wet = 0.5f * (left + right) + stereo_foldback_ * 0.5f * (left - right);

    // 8) Output mix.
    return input * (1.0f - mix_) + wet * mix_;
}