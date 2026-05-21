#include "effects/Compressor.h"

Compressor::Compressor() {
    updateCoefficients();
}

void Compressor::setSampleRate(float rate) {
    sample_rate_ = rate > 0 ? rate : 48000.0f;
    updateCoefficients();
}

void Compressor::updateCoefficients() {
    attack_coeff_ = exp(-1.0f / (attack_ms_ * sample_rate_ / 1000.0f));
    release_coeff_ = exp(-1.0f / (release_ms_ * sample_rate_ / 1000.0f));
}

void Compressor::setThreshold(float threshold_dB) {
    threshold_ = powf(10.0f, threshold_dB / 20.0f);
}

void Compressor::setRatio(float ratio) {
    ratio_ = std::max(1.0f, ratio);
}

void Compressor::setAttack(float attack_ms) {
    attack_ms_ = std::max(1.0f, attack_ms);
    updateCoefficients();
}

void Compressor::setRelease(float release_ms) {
    release_ms_ = std::max(1.0f, release_ms);
    updateCoefficients();
}

float Compressor::process(float input) {
    float input_abs = fabsf(input);

    // Envelope follower
    if (input_abs > envelope_) {
        envelope_ = attack_coeff_ * envelope_ + (1.0f - attack_coeff_) * input_abs;
    } else {
        envelope_ = release_coeff_ * envelope_ + (1.0f - release_coeff_) * input_abs;
    }

    float gain = 1.0f;
    if (envelope_ > threshold_) {
        // Gain reduction calculation
        float gain_reduction = threshold_ + (envelope_ - threshold_) / ratio_;
        gain = gain_reduction / envelope_; // Apply the reduction
    }

    // Store gain reduction in dB for the meter
    if (gain < 1.0f) {
        gain_reduction_db_.store(20.0f * log10(gain));
    } else {
        gain_reduction_db_.store(0.0f);
    }

    return input * gain;
}

float Compressor::getGainReductionDB() const {
    return gain_reduction_db_.load();
}