#/**
# * @file EQ.cpp
# * @brief Implements the EQ (equalizer) audio effect with low, mid, and high bands.
# */
#include "effects/EQ.h"
#include <algorithm>
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
    sample_rate_ = (rate > 0.0f) ? rate : 48000.0f;
    // About 25 ms smoothing time constant for gain moves.
    gain_smooth_coeff_ = 1.0f - std::exp(-1.0f / (0.025f * sample_rate_));
    // Gentle side estimate low-pass around ~1.5 kHz for pseudo M/S foldback.
    const float sideCut = 1500.0f;
    ms_side_lp_alpha_ = 1.0f - std::exp(-2.0f * PI * sideCut / sample_rate_);
    updateFilters();
}

void EQ::updateFilters() {
    low_shelf_.calculate_low_shelf(sample_rate_, 250.0f, low_gain_db_smoothed_);
    mid_peak_.calculate_peaking(sample_rate_, 1500.0f, mid_q_adapted_, mid_gain_db_smoothed_);
    high_shelf_.calculate_high_shelf(sample_rate_, 5000.0f, high_gain_db_smoothed_);
}

void EQ::setLowGain(float gain_db) { low_gain_db_ = gain_db; updateFilters(); }
void EQ::setMidGain(float gain_db) { mid_gain_db_ = gain_db; updateFilters(); }
void EQ::setHighGain(float gain_db) { high_gain_db_ = gain_db; updateFilters(); }
void EQ::setSaturationEnabled(bool enabled) { saturation_enabled_ = enabled; }
void EQ::setSaturationDrive(float drive) { saturation_drive_ = std::clamp(drive, 0.5f, 4.0f); }
void EQ::setMSAmount(float amount) {
    const float a = std::clamp(amount, 0.0f, 1.0f);
    ms_width_ = a;
    ms_foldback_ = 0.6f * a;
}

void EQ::updateSmoothingAndFilters() {
    low_gain_db_smoothed_ += gain_smooth_coeff_ * (low_gain_db_ - low_gain_db_smoothed_);
    mid_gain_db_smoothed_ += gain_smooth_coeff_ * (mid_gain_db_ - mid_gain_db_smoothed_);
    high_gain_db_smoothed_ += gain_smooth_coeff_ * (high_gain_db_ - high_gain_db_smoothed_);

    // Adaptive Q: keep boosts a bit tighter, cuts broader for musical behavior.
    const float midAbs = std::fabs(mid_gain_db_smoothed_);
    if (mid_gain_db_smoothed_ >= 0.0f) {
        mid_q_adapted_ = std::clamp(mid_q_base_ + 0.04f * midAbs, 0.7f, 1.8f);
    } else {
        mid_q_adapted_ = std::clamp(mid_q_base_ - 0.02f * midAbs, 0.6f, 1.3f);
    }

    updateFilters();
}

float EQ::processCore(float input) {
    float temp = input;
    temp = low_shelf_.process(temp);
    temp = mid_peak_.process(temp);
    temp = high_shelf_.process(temp);

    // Optional saturation after EQ for analog-ish tone when driven.
    const float boostAmount = std::max({0.0f, low_gain_db_smoothed_, mid_gain_db_smoothed_, high_gain_db_smoothed_});
    const bool satActive = saturation_enabled_ && boostAmount > 1.0f;
    if (satActive) {
        const float drive = saturation_drive_ + 0.025f * boostAmount;
        temp = std::tanh(drive * temp);
    }

    // Mono-safe M/S-style stage: derive side estimate and fold a small portion back to mono.
    ms_side_lp_state_ += ms_side_lp_alpha_ * (temp - ms_side_lp_state_);
    const float sideEstimate = temp - ms_side_lp_state_;
    const float mid = temp;
    const float left = mid + ms_width_ * sideEstimate;
    const float right = mid - ms_width_ * sideEstimate;
    temp = 0.5f * (left + right) + ms_foldback_ * 0.5f * (left - right);

    return temp;
}

float EQ::process(float input) {
    updateSmoothingAndFilters();

    if (!oversampling_enabled_) {
        return processCore(input);
    }

    // 2x oversampling (linear upsample, simple average downsample).
    const float x0 = 0.5f * (os_prev_input_ + input);
    const float x1 = input;
    const float y0 = processCore(x0);
    const float y1 = processCore(x1);
    os_prev_input_ = input;
    return 0.5f * (y0 + y1);
}