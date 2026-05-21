#include "effects/Limiter.h"

Limiter::Limiter() {
    updateCoefficients();
}

void Limiter::setSampleRate(float rate) {
    sample_rate_ = rate > 0 ? rate : 48000.0f;
    updateCoefficients();
}

void Limiter::updateCoefficients() {
    // Use a very fast 1ms attack
    attack_coeff_ = exp(-1.0f / (1.0f * sample_rate_ / 1000.0f));
    release_coeff_ = exp(-1.0f / (release_ms_ * sample_rate_ / 1000.0f));
}

void Limiter::setThreshold(float threshold_dB) {
    threshold_ = powf(10.0f, threshold_dB / 20.0f);
}

void Limiter::setRelease(float release_ms) {
    release_ms_ = std::max(1.0f, release_ms);
    updateCoefficients();
}

float Limiter::process(float input) {
    float input_abs = fabsf(input);

    // Envelope follower with fast attack
    if (input_abs > envelope_) {
        envelope_ = attack_coeff_ * envelope_ + (1.0f - attack_coeff_) * input_abs;
    } else {
        envelope_ = release_coeff_ * envelope_ + (1.0f - release_coeff_) * input_abs;
    }

    float target_gain = 1.0f;
    if (envelope_ > threshold_) {
        target_gain = threshold_ / envelope_;
    }

    // Smooth the gain change to prevent zipper noise
    gain_ = 0.99f * gain_ + 0.01f * target_gain;

    // Store gain reduction in dB for the meter
    if (gain_ < 1.0f) {
        gain_reduction_db_.store(20.0f * log10(gain_));
    } else {
        gain_reduction_db_.store(0.0f);
    }

    return input * gain_;
}

float Limiter::getGainReductionDB() const {
    return gain_reduction_db_.load();
}