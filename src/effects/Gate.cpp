#include "effects/Gate.h"

Gate::Gate() {
    updateCoefficients();
}

void Gate::setSampleRate(float rate) {
    sample_rate_ = rate > 0 ? rate : 48000.0f;
    updateCoefficients();
}

void Gate::updateCoefficients() {
    // Attack is very fast to open the gate quickly
    attack_coeff_ = exp(-1.0f / (1.0f * sample_rate_ / 1000.0f)); // 1ms attack
    release_coeff_ = exp(-1.0f / (release_ms_ * sample_rate_ / 1000.0f));
}

void Gate::setThreshold(float threshold_dB) {
    threshold_ = powf(10.0f, threshold_dB / 20.0f);
}

void Gate::setRelease(float release_ms) {
    release_ms_ = std::max(1.0f, release_ms);
    updateCoefficients();
}

float Gate::process(float input) {
    float input_abs = fabsf(input);

    // Envelope follower
    if (input_abs > envelope_) {
        envelope_ = attack_coeff_ * envelope_ + (1.0f - attack_coeff_) * input_abs;
    } else {
        envelope_ = release_coeff_ * envelope_ + (1.0f - release_coeff_) * input_abs;
    }

    if (gate_open_) {
        if (envelope_ < threshold_) {
            gate_open_ = false;
        }
    } else {
        if (envelope_ > threshold_) {
            gate_open_ = true;
        }
    }

    return gate_open_ ? input : 0.0f;
}