#include "effects/Reverb.h"
#include <cmath>

void Reverb::Comb::set(float feedback, float damp) {
    feedback_ = feedback;
    damp_ = damp;
}

float Reverb::Comb::process(float input) {
    float output = buffer[buf_idx_];
    filter_store_ = (output * (1.0f - damp_)) + (filter_store_ * damp_);
    buffer[buf_idx_] = input + (filter_store_ * feedback_);
    if (++buf_idx_ >= buf_size_) {
        buf_idx_ = 0;
    }
    return output;
}

void Reverb::AllPass::set(float feedback) {
    feedback_ = feedback;
}

float Reverb::AllPass::process(float input) {
    float buf_out = buffer[buf_idx_];
    float output = -input + buf_out;
    buffer[buf_idx_] = input + (buf_out * feedback_);
    if (++buf_idx_ >= buf_size_) {
        buf_idx_ = 0;
    }
    return output;
}

Reverb::Reverb() {
    // Initialize with some default comb filter sizes (primes are good)
    const int comb_sizes[] = {1117, 1187, 1277, 1351};
    combs_.resize(4);
    for (size_t i = 0; i < combs_.size(); ++i) {
        combs_[i].buffer.resize(comb_sizes[i]);
        combs_[i].buf_size_ = comb_sizes[i];
    }

    const int allpass_sizes[] = {557, 443, 347, 223};
    allpasses_.resize(4);
    for (size_t i = 0; i < allpasses_.size(); ++i) {
        allpasses_[i].buffer.resize(allpass_sizes[i]);
        allpasses_[i].buf_size_ = allpass_sizes[i];
    }
}

void Reverb::setSampleRate(float rate) {
    // Adjust delay lengths based on sample rate if needed
    // For simplicity, we'll use fixed sizes for now
    (void)rate;
}

void Reverb::setMix(float mix) {
    if (mix >= 0.0f && mix <= 1.0f) {
        mix_ = mix;
    }
}

float Reverb::process(float input) {
    float output = 0.0f;
    float input_mixed = input * 0.5f; // Attenuate input to prevent clipping

    // Pass through comb filters in parallel
    for (auto& comb : combs_) {
        output += comb.process(input_mixed);
    }

    // Pass through all-pass filters in series to increase echo density
    for (auto& allpass : allpasses_) {
        output = allpass.process(output);
    }

    // Simple mix
    return (input * (1.0f - mix_)) + (output * mix_);
}