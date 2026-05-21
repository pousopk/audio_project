#/**
# * @file EQ.cpp
# * @brief Implements the EQ (equalizer) audio effect with low, mid, and high bands.
# */
#include "effects/EQ.h"
#include <cmath>

constexpr float PI = 3.14159265358979323846f;

void EQ::Biquad::calculate_low_shelf(float sample_rate, float freq, float gain_db) {
    float A = powf(10.0f, gain_db / 40.0f);
    float w0 = 2.0f * PI * freq / sample_rate;
    float cos_w0 = cosf(w0);
    float sin_w0 = sinf(w0);
    float alpha = sin_w0 / 2.0f * sqrtf((A + 1.0f / A) * (1.0f / 0.707f - 1.0f) + 2.0f); // S = 0.707 for shelf

    float b0_ = A * ((A + 1.0f) - (A - 1.0f) * cos_w0 + 2.0f * sqrtf(A) * alpha);
    float b1_ = 2.0f * A * ((A - 1.0f) - (A + 1.0f) * cos_w0);
    float b2_ = A * ((A + 1.0f) - (A - 1.0f) * cos_w0 - 2.0f * sqrtf(A) * alpha);
    float a0_ = (A + 1.0f) + (A - 1.0f) * cos_w0 + 2.0f * sqrtf(A) * alpha;
    float a1_ = -2.0f * ((A - 1.0f) + (A + 1.0f) * cos_w0);
    float a2_ = (A + 1.0f) + (A - 1.0f) * cos_w0 - 2.0f * sqrtf(A) * alpha;

    a0 = b0_ / a0_; a1 = b1_ / a0_; a2 = b2_ / a0_; b1 = a1_ / a0_; b2 = a2_ / a0_;
}

void EQ::Biquad::calculate_peaking(float sample_rate, float freq, float q, float gain_db) {
    float A = powf(10.0f, gain_db / 40.0f);
    float w0 = 2.0f * PI * freq / sample_rate;
    float cos_w0 = cosf(w0);
    float sin_w0 = sinf(w0);
    float alpha = sin_w0 / (2.0f * q);

    float b0_ = 1.0f + alpha * A;
    float b1_ = -2.0f * cos_w0;
    float b2_ = 1.0f - alpha * A;
    float a0_ = 1.0f + alpha / A;
    float a1_ = -2.0f * cos_w0;
    float a2_ = 1.0f - alpha / A;

    a0 = b0_ / a0_; a1 = b1_ / a0_; a2 = b2_ / a0_; b1 = a1_ / a0_; b2 = a2_ / a0_;
}

void EQ::Biquad::calculate_high_shelf(float sample_rate, float freq, float gain_db) {
    float A = powf(10.0f, gain_db / 40.0f);
    float w0 = 2.0f * PI * freq / sample_rate;
    float cos_w0 = cosf(w0);
    float sin_w0 = sinf(w0);
    float alpha = sin_w0 / 2.0f * sqrtf((A + 1.0f / A) * (1.0f / 0.707f - 1.0f) + 2.0f);

    float b0_ = A * ((A + 1.0f) + (A - 1.0f) * cos_w0 + 2.0f * sqrtf(A) * alpha);
    float b1_ = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cos_w0);
    float b2_ = A * ((A + 1.0f) + (A - 1.0f) * cos_w0 - 2.0f * sqrtf(A) * alpha);
    float a0_ = (A + 1.0f) - (A - 1.0f) * cos_w0 + 2.0f * sqrtf(A) * alpha;
    float a1_ = 2.0f * ((A - 1.0f) - (A + 1.0f) * cos_w0);
    float a2_ = (A + 1.0f) - (A - 1.0f) * cos_w0 - 2.0f * sqrtf(A) * alpha;

    a0 = b0_ / a0_; a1 = b1_ / a0_; a2 = b2_ / a0_; b1 = a1_ / a0_; b2 = a2_ / a0_;
}

float EQ::Biquad::process(float input) {
    float out = a0 * input + a1 * x1 + a2 * x2 - b1 * y1 - b2 * y2;
    x2 = x1; x1 = input; y2 = y1; y1 = out;
    return out;
}

EQ::EQ() { updateFilters(); }

void EQ::setSampleRate(float rate) {
    sample_rate_ = rate;
    updateFilters();
}

void EQ::updateFilters() {
    low_shelf_.calculate_low_shelf(sample_rate_, 250.0f, low_gain_db_);
    mid_peak_.calculate_peaking(sample_rate_, 1500.0f, 1.0f, mid_gain_db_);
    high_shelf_.calculate_high_shelf(sample_rate_, 5000.0f, high_gain_db_);
}

void EQ::setLowGain(float gain_db) { low_gain_db_ = gain_db; updateFilters(); }
void EQ::setMidGain(float gain_db) { mid_gain_db_ = gain_db; updateFilters(); }
void EQ::setHighGain(float gain_db) { high_gain_db_ = gain_db; updateFilters(); }

float EQ::process(float input) {
    float temp = input;
    temp = low_shelf_.process(temp);
    temp = mid_peak_.process(temp);
    temp = high_shelf_.process(temp);
    return temp;
}