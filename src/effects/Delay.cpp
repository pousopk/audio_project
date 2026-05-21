#/**
# * @file Delay.cpp
# * @brief Implements the Delay audio effect for echo and time-based modulation.
# */
# 
#include "effects/Delay.h"
#include <algorithm>

Delay::Delay() {
    // Initialize with a max buffer size, e.g., 2 seconds
    buffer_.resize(48000 * 2, 0.0f);
    setDelayTime(500.0f); // Default delay time
}

void Delay::setSampleRate(float rate) {
    sample_rate_ = rate;
    buffer_.resize(static_cast<size_t>(rate * 2.0f), 0.0f); // 2 seconds max delay
    // Recalculate delay samples based on new sample rate
    setDelayTime(delay_samples_ * 1000.0f / sample_rate_);
}

void Delay::setDelayTime(float time_ms) {
    delay_samples_ = static_cast<int>(time_ms / 1000.0f * sample_rate_);
    delay_samples_ = std::clamp(delay_samples_, 0, static_cast<int>(buffer_.size() - 1));
}

void Delay::setFeedback(float feedback) {
    feedback_ = std::clamp(feedback, 0.0f, 0.99f); // Feedback >= 1 is unstable
}

void Delay::setMix(float mix) {
    mix_ = std::clamp(mix, 0.0f, 1.0f);
}

float Delay::process(float input) {
    int read_pos = (write_pos_ - delay_samples_ + buffer_.size()) % buffer_.size();
    float delayed_sample = buffer_[read_pos];

    buffer_[write_pos_] = input + (delayed_sample * feedback_);

    write_pos_ = (write_pos_ + 1) % buffer_.size();

    return (input * (1.0f - mix_)) + (delayed_sample * mix_);
}