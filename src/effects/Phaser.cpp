#include "effects/Phaser.h"
#include <algorithm>
#include <cmath>

constexpr float PI = 3.14159265358979323846f;

// --- AllPassFilter implementation ---
Phaser::AllPassFilter::AllPassFilter() = default;

float Phaser::AllPassFilter::process(float input) {
    float output = -input * coefficient_ + z1_;
    z1_ = output * coefficient_ + input;
    return output;
}

void Phaser::AllPassFilter::setCoefficient(float coeff) {
    coefficient_ = coeff;
}

// --- Phaser implementation ---
Phaser::Phaser() {
    filters_a_.resize(6);
    filters_b_.resize(6);
    setSampleRate(sample_rate_);
}

float Phaser::frequencyToCoefficient(float frequencyHz) const {
    const float f = std::clamp(frequencyHz, 40.0f, 0.45f * sample_rate_);
    const float w = PI * f / sample_rate_;
    const float t = std::tan(w);
    const float a = (t - 1.0f) / (t + 1.0f);
    return std::clamp(a, -0.98f, 0.98f);
}

void Phaser::updateStageCount() {
    // Requested stage count range 4-6, scale with depth.
    stage_count_ = 4 + static_cast<int>(std::round(std::clamp(depth_, 0.0f, 1.0f) * 2.0f));
    stage_count_ = std::clamp(stage_count_, 4, 6);
}

void Phaser::setSampleRate(float rate) {
    sample_rate_ = rate > 0 ? rate : 48000.0f;

    // Pre-filter coefficients.
    const float hpCut = 120.0f;
    const float lpCut = 7600.0f;
    pre_hp_alpha_ = 1.0f - std::exp(-2.0f * PI * hpCut / sample_rate_);
    pre_lp_alpha_ = 1.0f - std::exp(-2.0f * PI * lpCut / sample_rate_);

    // Feedback loop low-pass tone filter.
    const float fbCut = 5200.0f;
    feedback_lp_alpha_ = 1.0f - std::exp(-2.0f * PI * fbCut / sample_rate_);
}

void Phaser::setRate(float rate_hz) {
    rate_hz_ = std::clamp(rate_hz, 0.05f, 2.0f);
}

void Phaser::setDepth(float depth) {
    depth_ = std::clamp(depth, 0.0f, 1.0f);
    updateStageCount();
}

void Phaser::setFeedback(float feedback) {
    feedback_ = std::clamp(feedback, 0.0f, 0.9f);
}

void Phaser::setMix(float mix) {
    mix_ = std::clamp(mix, 0.0f, 1.0f);
}

float Phaser::process(float input) {
    // Pre-filter stage.
    pre_hp_state_ += pre_hp_alpha_ * (input - pre_hp_state_);
    const float hp = input - pre_hp_state_;
    pre_lp_state_ += pre_lp_alpha_ * (hp - pre_lp_state_);
    const float pre = pre_lp_state_;

    // Coefficient modulation from sweep range (300 Hz - 4 kHz).
    const float lfo_increment = 2.0f * PI * rate_hz_ / sample_rate_;
    lfo_phase_ += lfo_increment;
    if (lfo_phase_ >= 2.0f * PI) lfo_phase_ -= 2.0f * PI;

    const float lfoA = 0.5f * (1.0f + std::sin(lfo_phase_));
    const float lfoB = 0.5f * (1.0f + std::sin(lfo_phase_ + lfo_phase_offset_));
    const float sweepSpan = (sweep_max_hz_ - sweep_min_hz_) * (0.25f + 0.75f * depth_);

    const float centerA = sweep_min_hz_ + lfoA * sweepSpan;
    const float centerB = sweep_min_hz_ + lfoB * sweepSpan;

    // Feedback loop with saturation and filtering.
    feedback_lp_state_ += feedback_lp_alpha_ * (feedback_state_ - feedback_lp_state_);
    const float loop = std::tanh(feedback_sat_drive_ * feedback_lp_state_);

    float wetA = pre + feedback_ * loop;
    float wetB = pre + 0.85f * feedback_ * loop;

    for (int i = 0; i < stage_count_; ++i) {
        const float spreadA = 1.0f + 0.11f * static_cast<float>(i - (stage_count_ - 1) / 2.0f);
        const float spreadB = 1.0f + 0.09f * static_cast<float>(i - (stage_count_ - 1) / 2.0f);
        const float coeffA = frequencyToCoefficient(centerA * spreadA);
        const float coeffB = frequencyToCoefficient(centerB * spreadB);

        filters_a_[i].setCoefficient(coeffA);
        filters_b_[i].setCoefficient(coeffB);
        wetA = filters_a_[i].process(wetA);
        wetB = filters_b_[i].process(wetB);
    }

    // Stereo decorrelation collapsed mono-safely.
    const float wet = 0.62f * wetA + 0.38f * wetB;
    feedback_state_ = wet;

    return input * (1.0f - mix_) + wet * mix_;
}