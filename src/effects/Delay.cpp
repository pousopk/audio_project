#/**
# * @file Delay.cpp
# * @brief Implements the Delay audio effect for echo and time-based modulation.
# */
# 
#include "effects/Delay.h"
#include <algorithm>
#include <cmath>

namespace {
constexpr float kPi = 3.14159265358979323846f;
}

Delay::Delay() {
    buffer_.assign(static_cast<size_t>(sample_rate_ * 2.5f), 0.0f);
    diffusion_buffer_.assign(static_cast<size_t>(sample_rate_ * 0.03f), 0.0f);
    setDelayTime(delay_time_ms_);
    // One-pole LP in feedback loop around ~3.2kHz.
    const float cutoff = 3200.0f;
    feedback_lpf_alpha_ = 1.0f - std::exp(-2.0f * kPi * cutoff / sample_rate_);
}

void Delay::setSampleRate(float rate) {
    sample_rate_ = (rate > 0.0f) ? rate : 48000.0f;
    buffer_.assign(static_cast<size_t>(sample_rate_ * 2.5f), 0.0f);
    diffusion_buffer_.assign(static_cast<size_t>(sample_rate_ * 0.03f), 0.0f);
    write_pos_ = 0;
    diffusion_write_pos_ = 0;
    feedback_lpf_state_ = 0.0f;

    const float cutoff = 3200.0f;
    feedback_lpf_alpha_ = 1.0f - std::exp(-2.0f * kPi * cutoff / sample_rate_);

    setDelayTime(delay_time_ms_);
}

void Delay::setDelayTime(float time_ms) {
    delay_time_ms_ = std::clamp(time_ms, 1.0f, 1800.0f);
    delay_samples_ = (delay_time_ms_ / 1000.0f) * sample_rate_;
    delay_samples_ = std::clamp(delay_samples_, 1.0f, static_cast<float>(buffer_.size() - 2));

    // Deeper modulation for longer times, still subtle.
    mod_depth_ms_ = std::clamp(1.0f + 0.01f * delay_time_ms_, 1.5f, 9.0f);
}

void Delay::setFeedback(float feedback) {
    feedback_ = std::clamp(feedback, 0.0f, 0.95f);
}

void Delay::setMix(float mix) {
    mix_ = std::clamp(mix, 0.0f, 1.0f);
}

float Delay::readDelayLinear(float delaySamples) const {
    if (buffer_.empty()) return 0.0f;

    const int size = static_cast<int>(buffer_.size());
    float readPos = static_cast<float>(write_pos_) - delaySamples;
    while (readPos < 0.0f) readPos += static_cast<float>(size);
    while (readPos >= static_cast<float>(size)) readPos -= static_cast<float>(size);

    const int i0 = static_cast<int>(readPos);
    const int i1 = (i0 + 1) % size;
    const float frac = readPos - static_cast<float>(i0);
    return buffer_[i0] * (1.0f - frac) + buffer_[i1] * frac;
}

float Delay::randomSigned() {
    rand_state_ = 1664525u * rand_state_ + 1013904223u;
    const float u = static_cast<float>(rand_state_ & 0x00FFFFFFu) / static_cast<float>(0x01000000u);
    return 2.0f * u - 1.0f;
}

float Delay::process(float input) {
    if (buffer_.empty()) return input;

    // Slow random modulation target updates.
    if (--mod_counter_samples_ <= 0) {
        mod_target_ms_ = mod_depth_ms_ * randomSigned();
        const float holdMs = 250.0f + 700.0f * (0.5f * (randomSigned() + 1.0f));
        mod_counter_samples_ = static_cast<int>((holdMs / 1000.0f) * sample_rate_);
    }
    mod_current_ms_ += 0.0015f * (mod_target_ms_ - mod_current_ms_);

    // Fractional modulated taps.
    const float tap1Delay = std::clamp(delay_samples_ + mod_current_ms_ * sample_rate_ / 1000.0f,
                                       1.0f,
                                       static_cast<float>(buffer_.size() - 2));
    const float tap2Delay = std::clamp(delay_samples_ + decorrelation_offset_ms_ * sample_rate_ / 1000.0f
                                       - mod_current_ms_ * sample_rate_ / 1000.0f,
                                       1.0f,
                                       static_cast<float>(buffer_.size() - 2));

    const float tap1 = readDelayLinear(tap1Delay);
    const float tap2 = readDelayLinear(tap2Delay);

    // Mono-safe stereo decorrelation approximation.
    const float decorrelated = 0.62f * tap1 + 0.38f * tap2;

    // Gentle diffusion all-pass stage.
    float diffused = decorrelated;
    if (!diffusion_buffer_.empty()) {
        const int dsize = static_cast<int>(diffusion_buffer_.size());
        const float apRead = diffusion_buffer_[diffusion_write_pos_];
        diffused = -diffusion_g_ * decorrelated + apRead;
        diffusion_buffer_[diffusion_write_pos_] = decorrelated + diffusion_g_ * diffused;
        diffusion_write_pos_ = (diffusion_write_pos_ + 1) % dsize;
    }

    // Feedback loop: low-pass then subtle saturation.
    feedback_lpf_state_ += feedback_lpf_alpha_ * (diffused - feedback_lpf_state_);
    const float loopSat = std::tanh(loop_saturation_drive_ * feedback_lpf_state_);

    buffer_[write_pos_] = input + feedback_ * loopSat;
    write_pos_ = (write_pos_ + 1) % static_cast<int>(buffer_.size());

    return input * (1.0f - mix_) + diffused * mix_;
}