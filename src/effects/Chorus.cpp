#/**
# * @file Chorus.cpp
# * @brief Implements the Chorus audio effect for thickening and modulating sound.
# */
#include "effects/Chorus.h"
#include <algorithm>
#include <array>
#include <cmath>

constexpr float PI = 3.14159265358979323846f;

Chorus::Chorus() {
    delay_buffer_.resize(static_cast<size_t>(sample_rate_ * 0.08f), 0.0f);
    voices_.resize(kNumVoices);

    // Evenly spread voice phases and small delay biases for width.
    for (int i = 0; i < kNumVoices; ++i) {
        voices_[i].phaseOffset = (2.0f * PI * static_cast<float>(i)) / static_cast<float>(kNumVoices);
        voices_[i].delayBiasMs = -1.5f + 3.0f * (static_cast<float>(i) / static_cast<float>(kNumVoices - 1));
    }

    updateLFO();
}

void Chorus::setSampleRate(float rate) {
    sample_rate_ = rate > 0 ? rate : 48000.0f;
    delay_buffer_.assign(static_cast<size_t>(sample_rate_ * 0.08f), 0.0f);
    write_pos_ = 0;
    pre_filter_state_ = 0.0f;
    feedback_state_ = 0.0f;
    for (auto& v : voices_) {
        v.filterState = 0.0f;
    }
    updateLFO();
}

void Chorus::updateLFO() {
    // LFO increment calculation is done in process() for variable rate
}

float Chorus::onePoleAlpha(float cutoffHz) const {
    const float c = std::clamp(cutoffHz, 20.0f, 0.45f * sample_rate_);
    return 1.0f - std::exp(-2.0f * PI * c / sample_rate_);
}

float Chorus::readDelayLinear(float delaySamples) const {
    if (delay_buffer_.empty()) return 0.0f;

    const int size = static_cast<int>(delay_buffer_.size());
    float readPos = static_cast<float>(write_pos_) - delaySamples;
    while (readPos < 0.0f) readPos += static_cast<float>(size);
    while (readPos >= static_cast<float>(size)) readPos -= static_cast<float>(size);

    const int i0 = static_cast<int>(readPos);
    const int i1 = (i0 + 1) % size;
    const float frac = readPos - static_cast<float>(i0);
    return delay_buffer_[i0] * (1.0f - frac) + delay_buffer_[i1] * frac;
}

void Chorus::setRate(float rate_hz) {
    rate_hz_ = std::clamp(rate_hz, 0.01f, 8.0f);
}

void Chorus::setDepth(float depth_ms) {
    depth_ms_ = std::clamp(depth_ms, 0.0f, 20.0f);
}

void Chorus::setMix(float mix) {
    mix_ = std::clamp(mix, 0.0f, 1.0f);
}

float Chorus::process(float input) {
    if (delay_buffer_.empty()) {
        return input;
    }

    const int delaySize = static_cast<int>(delay_buffer_.size());
    if (write_pos_ < 0 || write_pos_ >= delaySize) {
        write_pos_ = 0;
    }

    // Stage 1: pre-filter.
    const float preAlpha = onePoleAlpha(6000.0f);
    pre_filter_state_ += preAlpha * (input - pre_filter_state_);
    const float preFiltered = pre_filter_state_;

    // Stage 6 input (feedback fed into the delay writer).
    const float writeSample = preFiltered + feedback_state_ * feedback_;
    delay_buffer_[write_pos_] = writeSample;

    // LFO update.
    const float lfoIncrement = 2.0f * PI * rate_hz_ / sample_rate_;
    lfo_phase_ += lfoIncrement;
    if (lfo_phase_ >= 2.0f * PI) {
        lfo_phase_ -= 2.0f * PI;
    }

    // Stage 2..5: voices, per-voice filter, decorrelation.
    float wetSum = 0.0f;
    float feedbackTap = 0.0f;
    for (int i = 0; i < kNumVoices; ++i) {
        auto& v = voices_[i];
        const float lfo = std::sin(lfo_phase_ + v.phaseOffset);

        float delayMs = base_delay_ms_ + v.delayBiasMs + depth_ms_ * (0.5f + 0.5f * lfo);
        delayMs = std::clamp(delayMs, 1.0f, 45.0f);
        float delaySamples = delayMs * sample_rate_ / 1000.0f;
        delaySamples = std::clamp(delaySamples, 1.0f, static_cast<float>(delaySize - 2));

        const float delayed = readDelayLinear(delaySamples);

        // Stage 3: per-voice filtering (slightly different cutoff per voice).
        const float voiceCutoff = 1400.0f + 600.0f * static_cast<float>(i);
        const float voiceAlpha = onePoleAlpha(voiceCutoff);
        v.filterState += voiceAlpha * (delayed - v.filterState);
        const float filtered = v.filterState;

        // Stage 4: decorrelation (alternating polarity + tiny weighting skew).
        const float polarity = (i % 2 == 0) ? 1.0f : -1.0f;
        const float weight = 0.7f + 0.3f * static_cast<float>(i) / static_cast<float>(kNumVoices - 1);
        const float decorrelated = filtered * polarity * weight;

        wetSum += decorrelated;
        feedbackTap += filtered;
    }

    float wet = wetSum / static_cast<float>(kNumVoices);

    // Stage 6: optional feedback.
    const float fbTarget = feedbackTap / static_cast<float>(kNumVoices);
    feedback_state_ = 0.85f * feedback_state_ + 0.15f * fbTarget;
    feedback_state_ = std::clamp(feedback_state_, -1.0f, 1.0f);

    // Stage 7: saturation.
    wet = std::tanh(saturation_drive_ * wet);

    write_pos_ = (write_pos_ + 1) % delaySize;

    // Stage 8: mix.
    return input * (1.0f - mix_) + wet * mix_;
}