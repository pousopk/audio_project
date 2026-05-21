#include "effects/Tremolo.h"
#include <cmath>

constexpr float PI = 3.14159265358979323846f;

Tremolo::Tremolo() = default;

void Tremolo::setSampleRate(float rate) {
    sample_rate_ = rate > 0 ? rate : 48000.0f;
}

void Tremolo::setRate(float rate_hz) {
    rate_hz_ = rate_hz;
}

void Tremolo::setDepth(float depth) {
    depth_ = depth;
}

float Tremolo::process(float input) {
    // Calculate LFO
    float lfo_increment = 2.0f * PI * rate_hz_ / sample_rate_;
    lfo_phase_ += lfo_increment;
    if (lfo_phase_ >= 2.0f * PI) {
        lfo_phase_ -= 2.0f * PI;
    }
    // Sine LFO from 0 to 1
    float lfo_out = (1.0f + sin(lfo_phase_)) / 2.0f;

    // Calculate modulation amount
    float mod = 1.0f - (depth_ * lfo_out);

    return input * mod;
}