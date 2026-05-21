#include "effects/Flanger.h"
#include <cmath>

constexpr float PI = 3.14159265358979323846f;

Flanger::Flanger() {
    // Max delay of 20ms for flanger effects
    delay_buffer_.resize(48000 * 0.02, 0.0f);
}

void Flanger::setSampleRate(float rate) {
    sample_rate_ = rate > 0 ? rate : 48000.0f;
    delay_buffer_.resize(static_cast<size_t>(sample_rate_ * 0.02), 0.0f);
}

void Flanger::setRate(float rate_hz) {
    rate_hz_ = rate_hz;
}

void Flanger::setDepth(float depth_ms) {
    depth_ms_ = depth_ms;
}

void Flanger::setFeedback(float feedback) {
    feedback_ = feedback;
}

void Flanger::setMix(float mix) {
    mix_ = mix;
}

float Flanger::process(float input) {
    // Calculate LFO
    float lfo_increment = 2.0f * PI * rate_hz_ / sample_rate_;
    lfo_phase_ += lfo_increment;
    if (lfo_phase_ >= 2.0f * PI) {
        lfo_phase_ -= 2.0f * PI;
    }
    float lfo_out = sin(lfo_phase_);

    // Calculate delay time from LFO (flanger has a shorter base delay)
    float base_delay_ms = 1.0f; // Small base delay for flanger
    float delay_samples = (base_delay_ms + depth_ms_ * (1.0f + lfo_out) / 2.0f) * sample_rate_ / 1000.0f;
    float read_pos = write_pos_ - delay_samples;
    while (read_pos < 0) read_pos += delay_buffer_.size();

    // Simple linear interpolation for fractional delay
    int read_pos_int = static_cast<int>(read_pos);
    float frac = read_pos - read_pos_int;
    float delayed_sample = (1.0f - frac) * delay_buffer_[read_pos_int] + frac * delay_buffer_[(read_pos_int + 1) % delay_buffer_.size()];

    // Store current sample with feedback
    delay_buffer_[write_pos_] = input + delayed_sample * feedback_;
    write_pos_ = (write_pos_ + 1) % delay_buffer_.size();

    return input * (1.0f - mix_) + delayed_sample * mix_;
}