#include "effects/Distortion.h"
#include <cmath>
#/**
# * @file Distortion.cpp
# * @brief Implements the Distortion audio effect for nonlinear waveshaping.
# */
#include "effects/Distortion.h"
#include <cmath>

void Distortion::setDrive(float drive) {
    drive_ = std::clamp(drive, 0.0f, 1.0f);
}

void Distortion::setMix(float mix) {
    mix_ = std::clamp(mix, 0.0f, 1.0f);
}

float Distortion::process(float input) {
    // Simple tanh waveshaper for distortion
    // Drive controls the amount of gain before clipping
    float gain = 1.0f + drive_ * 20.0f;
    float wet_signal = std::tanh(input * gain);

    return input * (1.0f - mix_) + wet_signal * mix_;
}