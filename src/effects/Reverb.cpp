#include "effects/Reverb.h"
#include <algorithm>
#include <cmath>

namespace {
constexpr float kPi = 3.14159265358979323846f;
}

void Reverb::Comb::set(float sampleRate,
                       float feedback,
                       float dampHz,
                       float toneHz,
                       float baseDelaySamples,
                       float modRateHz,
                       float modDepthSamples) {
    sample_rate_ = sampleRate;
    feedback_ = feedback;
    base_delay_samples_ = baseDelaySamples;
    mod_rate_hz_ = modRateHz;
    mod_depth_samples_ = modDepthSamples;

    damp_alpha_ = 1.0f - std::exp(-2.0f * kPi * dampHz / sample_rate_);
    tone_alpha_ = 1.0f - std::exp(-2.0f * kPi * toneHz / sample_rate_);
}

float Reverb::Comb::readLinear(float delaySamples) const {
    if (buffer.empty()) return 0.0f;
    const int size = static_cast<int>(buffer.size());
    float readPos = static_cast<float>(write_pos_) - delaySamples;
    while (readPos < 0.0f) readPos += static_cast<float>(size);
    while (readPos >= static_cast<float>(size)) readPos -= static_cast<float>(size);

    const int i0 = static_cast<int>(readPos);
    const int i1 = (i0 + 1) % size;
    const float frac = readPos - static_cast<float>(i0);
    return buffer[i0] * (1.0f - frac) + buffer[i1] * frac;
}

float Reverb::Comb::process(float input) {
    if (buffer.empty()) return input;

    if (--mod_hold_counter_ <= 0) {
        rand_state_ = 1664525u * rand_state_ + 1013904223u;
        const float u = static_cast<float>(rand_state_ & 0x00FFFFFFu) / static_cast<float>(0x01000000u);
        mod_target_ = 2.0f * u - 1.0f;
        mod_hold_counter_ = static_cast<int>(sample_rate_ * 0.7f);
    }
    mod_current_ += 0.0012f * (mod_target_ - mod_current_);

    mod_phase_ += 2.0f * kPi * mod_rate_hz_ / sample_rate_;
    if (mod_phase_ >= 2.0f * kPi) mod_phase_ -= 2.0f * kPi;
    const float mod = 0.65f * std::sin(mod_phase_) + 0.35f * mod_current_;
    const float delaySamples = std::clamp(base_delay_samples_ + mod_depth_samples_ * mod,
                                          1.0f,
                                          static_cast<float>(buffer.size() - 2));

    const float delayed = readLinear(delaySamples);

    // HF damping + additional tone shaping in feedback path.
    damp_state_ += damp_alpha_ * (delayed - damp_state_);
    tone_state_ += tone_alpha_ * (damp_state_ - tone_state_);
    const float filtered = 0.7f * damp_state_ + 0.3f * tone_state_;

    const float fb = std::tanh(1.08f * filtered);
    buffer[write_pos_] = input + feedback_ * fb;
    write_pos_ = (write_pos_ + 1) % static_cast<int>(buffer.size());

    return delayed;
}

void Reverb::AllPass::set(float feedback) {
    feedback_ = feedback;
}

float Reverb::AllPass::process(float input) {
    if (buffer.empty()) return input;
    float buf_out = buffer[write_pos_];
    float output = -input + buf_out;
    buffer[write_pos_] = input + (buf_out * feedback_);
    write_pos_ = (write_pos_ + 1) % static_cast<int>(buffer.size());
    return output;
}

Reverb::Reverb() {
    early_tap_ms_ = {6.0f, 12.0f, 19.0f, 27.0f, 36.0f};
    initNetwork();
}

void Reverb::initNetwork() {
    predelay_buffer_.assign(static_cast<size_t>(sample_rate_ * 0.08f), 0.0f);
    predelay_write_pos_ = 0;

    // 6 combs (within requested 4-8) with gentle spread for decorrelation.
    const float combBaseMs[] = {29.7f, 33.1f, 36.7f, 40.3f, 43.9f, 47.6f};
    const float combModRates[] = {0.05f, 0.07f, 0.09f, 0.06f, 0.08f, 0.10f};
    combs_.resize(6);
    for (int i = 0; i < 6; ++i) {
        const int sz = std::max(8, static_cast<int>((combBaseMs[i] / 1000.0f) * sample_rate_) + 4);
        combs_[i].buffer.assign(static_cast<size_t>(sz), 0.0f);
        combs_[i].write_pos_ = 0;
        combs_[i].damp_state_ = 0.0f;
        combs_[i].tone_state_ = 0.0f;
        combs_[i].mod_phase_ = 0.4f * static_cast<float>(i);
        combs_[i].mod_hold_counter_ = static_cast<int>(sample_rate_ * (0.3f + 0.1f * static_cast<float>(i)));
        const float baseDelaySamples = (combBaseMs[i] / 1000.0f) * sample_rate_;
        combs_[i].set(sample_rate_, 0.80f, hf_damp_hz_, tone_hz_, baseDelaySamples, combModRates[i], 2.2f);
    }

    // 3 all-pass filters per branch (within requested 2-4).
    const float diffAMs[] = {8.1f, 5.4f, 3.7f};
    const float diffBMs[] = {7.6f, 4.9f, 3.3f};
    diffusers_a_.resize(3);
    diffusers_b_.resize(3);
    for (int i = 0; i < 3; ++i) {
        const int aSize = std::max(4, static_cast<int>((diffAMs[i] / 1000.0f) * sample_rate_));
        diffusers_a_[i].buffer.assign(static_cast<size_t>(aSize), 0.0f);
        diffusers_a_[i].write_pos_ = 0;
        diffusers_a_[i].set(0.58f);

        const int bSize = std::max(4, static_cast<int>((diffBMs[i] / 1000.0f) * sample_rate_));
        diffusers_b_[i].buffer.assign(static_cast<size_t>(bSize), 0.0f);
        diffusers_b_[i].write_pos_ = 0;
        diffusers_b_[i].set(0.56f);
    }
}

void Reverb::setSampleRate(float rate) {
    sample_rate_ = (rate > 0.0f) ? rate : 48000.0f;
    initNetwork();
}

void Reverb::setMix(float mix) {
    mix_ = std::clamp(mix, 0.0f, 1.0f);
}

float Reverb::readFromBuffer(const std::vector<float>& buffer, int writePos, float delaySamples) const {
    if (buffer.empty()) return 0.0f;
    const int size = static_cast<int>(buffer.size());
    float readPos = static_cast<float>(writePos) - delaySamples;
    while (readPos < 0.0f) readPos += static_cast<float>(size);
    while (readPos >= static_cast<float>(size)) readPos -= static_cast<float>(size);
    const int i0 = static_cast<int>(readPos);
    const int i1 = (i0 + 1) % size;
    const float frac = readPos - static_cast<float>(i0);
    return buffer[i0] * (1.0f - frac) + buffer[i1] * frac;
}

float Reverb::process(float input) {
    if (predelay_buffer_.empty()) {
        return input;
    }

    // Pre-delay.
    predelay_buffer_[predelay_write_pos_] = input;
    const float predelaySamples = (predelay_ms_ / 1000.0f) * sample_rate_;
    const float predelayed = readFromBuffer(predelay_buffer_, predelay_write_pos_, predelaySamples);

    // Early reflection taps.
    float early = 0.0f;
    for (size_t i = 0; i < early_tap_ms_.size(); ++i) {
        const float dSamp = (early_tap_ms_[i] / 1000.0f) * sample_rate_;
        const float tap = readFromBuffer(predelay_buffer_, predelay_write_pos_, dSamp);
        early += (0.20f - 0.02f * static_cast<float>(i)) * tap;
    }

    predelay_write_pos_ = (predelay_write_pos_ + 1) % static_cast<int>(predelay_buffer_.size());

    // Diffusion: two decorrelated all-pass chains.
    float diffA = predelayed + 0.7f * early;
    float diffB = predelayed + 0.5f * early;
    for (auto& ap : diffusers_a_) diffA = ap.process(diffA);
    for (auto& ap : diffusers_b_) diffB = ap.process(diffB);

    // Parallel comb decay engine.
    float wetA = 0.0f;
    float wetB = 0.0f;
    for (size_t i = 0; i < combs_.size(); ++i) {
        const float inA = diffA + 0.15f * diffB;
        const float inB = diffB + 0.15f * diffA;
        const float cA = combs_[i].process((i % 2 == 0) ? inA : inB);
        if (i % 2 == 0) wetA += cA;
        else wetB += cA;
    }

    wetA *= 0.35f;
    wetB *= 0.35f;

    // Stereo diffusion matrix (decorrelated branches folded mono-safely).
    const float left = 0.72f * wetA + 0.28f * wetB;
    const float right = 0.72f * wetB + 0.28f * wetA;
    const float wet = 0.5f * (left + right) + 0.12f * 0.5f * (left - right);

    return input * (1.0f - mix_) + wet * mix_;
}