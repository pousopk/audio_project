#include "effects/Flanger.h"
#include <algorithm>
#include <cmath>

constexpr float PI = 3.14159265358979323846f;

Flanger::Flanger() {
    delay_buffer_.assign(static_cast<size_t>(sample_rate_ * 0.02f), 0.0f);
    updateCoefficients();
}

void Flanger::setSampleRate(float rate) {
    sample_rate_ = rate > 0 ? rate : 48000.0f;
    delay_buffer_.assign(static_cast<size_t>(sample_rate_ * 0.02f), 0.0f);
    write_pos_ = 0;
    pre_hp_state_ = 0.0f;
    pre_lp_state_ = 0.0f;
    fb_lp_state_ = 0.0f;
    updateCoefficients();
}

void Flanger::setRate(float rate_hz) {
    rate_hz_ = std::clamp(rate_hz, 0.02f, 5.0f);
}

void Flanger::setDepth(float depth_ms) {
    depth_ms_ = std::clamp(depth_ms, 0.0f, 8.0f);
}

void Flanger::setFeedback(float feedback) {
    feedback_ = std::clamp(feedback, 0.0f, 0.9f);
}

void Flanger::setMix(float mix) {
    mix_ = std::clamp(mix, 0.0f, 1.0f);
}

void Flanger::updateCoefficients() {
    // Pre-filter: tighten lows and tame highs before combing.
    const float hpCut = 120.0f;
    const float lpCut = 7000.0f;
    pre_hp_alpha_ = 1.0f - std::exp(-2.0f * PI * hpCut / sample_rate_);
    pre_lp_alpha_ = 1.0f - std::exp(-2.0f * PI * lpCut / sample_rate_);

    // Feedback loop LP around requested 6-10kHz window.
    const float fbCut = 8000.0f;
    fb_lp_alpha_ = 1.0f - std::exp(-2.0f * PI * fbCut / sample_rate_);
}

float Flanger::readDelayLinear(float delaySamples) const {
    if (delay_buffer_.empty()) return 0.0f;

    const int size = static_cast<int>(delay_buffer_.size());
    float readPos = static_cast<float>(write_pos_) - delaySamples;
    while (readPos < 0.0f) readPos += static_cast<float>(size);
    while (readPos >= static_cast<float>(size)) readPos -= static_cast<float>(size);

    const int i0 = static_cast<int>(readPos);
    const int i1 = (i0 + 1) % size;
    const float frac = readPos - static_cast<float>(i0);
    return delay_buffer_[i0] * (1.0f - frac) + delay_buffer_[i1] * frac;
}

float Flanger::process(float input) {
    if (delay_buffer_.empty()) return input;

    // Input pre-filter (HP/LP tone shaping).
    pre_hp_state_ += pre_hp_alpha_ * (input - pre_hp_state_);
    const float hp = input - pre_hp_state_;
    pre_lp_state_ += pre_lp_alpha_ * (hp - pre_lp_state_);
    const float pre = pre_lp_state_;

    // Fractional modulated delay with stereo phase offset (mono-safe dual tap).
    const float lfo_increment = 2.0f * PI * rate_hz_ / sample_rate_;
    lfo_phase_ += lfo_increment;
    if (lfo_phase_ >= 2.0f * PI) lfo_phase_ -= 2.0f * PI;

    const float lfoL = std::sin(lfo_phase_);
    const float lfoR = std::sin(lfo_phase_ + lfo_phase_offset_rad_);
    const float delayMsL = std::clamp(base_delay_ms_ + depth_ms_ * lfoL, 0.1f, 10.0f);
    const float delayMsR = std::clamp(base_delay_ms_ + depth_ms_ * lfoR, 0.1f, 10.0f);
    const float tapL = readDelayLinear(delayMsL * sample_rate_ / 1000.0f);
    const float tapR = readDelayLinear(delayMsR * sample_rate_ / 1000.0f);

    // Stereo decorrelation collapsed mono-safely.
    const float decorrelated = 0.6f * tapL + 0.4f * tapR;

    // Feedback loop: lowpass + saturation + slight compression.
    fb_lp_state_ += fb_lp_alpha_ * (decorrelated - fb_lp_state_);
    float loop = std::tanh(loop_sat_drive_ * fb_lp_state_);
    const float a = std::fabs(loop);
    if (a > loop_comp_threshold_) {
        const float compressed = loop_comp_threshold_ + (a - loop_comp_threshold_) * loop_comp_ratio_;
        loop = std::copysign(compressed, loop);
    }

    delay_buffer_[write_pos_] = pre + feedback_ * loop;
    write_pos_ = (write_pos_ + 1) % static_cast<int>(delay_buffer_.size());

    return input * (1.0f - mix_) + decorrelated * mix_;
}