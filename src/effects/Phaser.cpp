#include "effects/Phaser.h"
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
    // Create a 6-stage phaser
    filters_.resize(6);
}

void Phaser::setSampleRate(float rate) {
    sample_rate_ = rate > 0 ? rate : 48000.0f;
}

void Phaser::setRate(float rate_hz) {
    rate_hz_ = rate_hz;
}

void Phaser::setDepth(float depth) {
    depth_ = depth;
}

void Phaser::setFeedback(float feedback) {
    feedback_ = feedback;
}

void Phaser::setMix(float mix) {
    mix_ = mix;
}

float Phaser::process(float input) {
    // Calculate LFO
    float lfo_increment = 2.0f * PI * rate_hz_ / sample_rate_;
    lfo_phase_ += lfo_increment;
    if (lfo_phase_ >= 2.0f * PI) {
        lfo_phase_ -= 2.0f * PI;
    }
    float lfo_out = (1.0f + sin(lfo_phase_)) / 2.0f; // 0 to 1

    // Modulate filter coefficients
    float d = depth_ * lfo_out;
    for (auto& filter : filters_) {
        filter.setCoefficient(d);
    }

    // Process through all-pass filters with feedback
    float wet_signal = input + last_out_ * feedback_;
    for (auto& filter : filters_) {
        wet_signal = filter.process(wet_signal);
    }
    last_out_ = wet_signal;

    return input * (1.0f - mix_) + wet_signal * mix_;
}