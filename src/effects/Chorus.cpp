#/**
# * @file Chorus.cpp
# * @brief Implements the Chorus audio effect for thickening and modulating sound.
# */
#include "effects/Chorus.h"
#include <cmath>

constexpr float PI = 3.14159265358979323846f;

Chorus::Chorus() {
    // Max delay of 50ms for chorus/flanger effects
    delay_buffer_.resize(48000 * 0.05, 0.0f);
    updateLFO();
}

void Chorus::setSampleRate(float rate) {
    sample_rate_ = rate > 0 ? rate : 48000.0f;
    delay_buffer_.resize(static_cast<size_t>(sample_rate_ * 0.05), 0.0f);
    updateLFO();
}

void Chorus::updateLFO() {
    // LFO increment calculation is done in process() for variable rate
}

void Chorus::setRate(float rate_hz) {
    rate_hz_ = rate_hz;
}

void Chorus::setDepth(float depth_ms) {
    depth_ms_ = depth_ms;
}

void Chorus::setMix(float mix) {
    mix_ = mix;
}

float Chorus::process(float input) {
    // Store current sample
    delay_buffer_[write_pos_] = input;

    // Calculate LFO
    float lfo_increment = 2.0f * PI * rate_hz_ / sample_rate_;
    lfo_phase_ += lfo_increment;
    if (lfo_phase_ >= 2.0f * PI) {
        lfo_phase_ -= 2.0f * PI;
    }
    float lfo_out = sin(lfo_phase_);

    // Calculate delay time from LFO
    float delay_samples = (depth_ms_ / 1000.0f) * sample_rate_ * (1.0f + lfo_out) / 2.0f;
    float read_pos = write_pos_ - delay_samples;
    while (read_pos < 0) read_pos += delay_buffer_.size();

    // Simple linear interpolation for fractional delay
    int read_pos_int = static_cast<int>(read_pos);
    float frac = read_pos - read_pos_int;
    float delayed_sample = (1.0f - frac) * delay_buffer_[read_pos_int] + frac * delay_buffer_[(read_pos_int + 1) % delay_buffer_.size()];

    write_pos_ = (write_pos_ + 1) % delay_buffer_.size();

    return input * (1.0f - mix_) + delayed_sample * mix_;
}