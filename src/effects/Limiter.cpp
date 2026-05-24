#include "effects/Limiter.h"

namespace {
constexpr float kEps = 1.0e-9f;
}

Limiter::Limiter() {
    setThreshold(threshold_db_);
    updateCoefficients();
}

void Limiter::setSampleRate(float rate) {
    sample_rate_ = rate > 0 ? rate : 48000.0f;
    updateCoefficients();
}

float Limiter::msToCoeff(float ms) const {
    const float m = std::max(0.05f, ms);
    return std::exp(-1.0f / (m * sample_rate_ / 1000.0f));
}

void Limiter::updateCoefficients() {
    oversample_factor_ = std::clamp(oversample_factor_, 4, 8);

    lookahead_samples_ = std::max(1, static_cast<int>(lookahead_ms_ * sample_rate_ / 1000.0f));
    lookahead_buffer_.assign(static_cast<size_t>(lookahead_samples_ + 2), 0.0f);
    lookahead_write_pos_ = 0;

    attack_coeff_ = msToCoeff(0.5f);
    release_coeff_fast_ = msToCoeff(std::clamp(0.45f * release_ms_, 20.0f, 180.0f));
    release_coeff_slow_ = msToCoeff(std::clamp(1.6f * release_ms_, 80.0f, 500.0f));
    detector_release_coeff_ = msToCoeff(35.0f);

    const float linkHz = 1400.0f;
    link_lp_alpha_ = 1.0f - std::exp(-2.0f * 3.14159265358979323846f * linkHz / sample_rate_);

    output_ceiling_ = std::pow(10.0f, output_ceiling_dbtp_ / 20.0f);
}

void Limiter::setThreshold(float threshold_dB) {
    threshold_db_ = std::clamp(threshold_dB, -20.0f, 0.0f);
    threshold_ = std::pow(10.0f, threshold_db_ / 20.0f);
}

void Limiter::setRelease(float release_ms) {
    release_ms_ = std::clamp(release_ms, 50.0f, 300.0f);
    updateCoefficients();
}

float Limiter::process(float input) {
    if (lookahead_buffer_.empty()) {
        updateCoefficients();
    }

    // Lookahead buffer (delays program path while detector sees current signal).
    float delayedInput = lookahead_buffer_[lookahead_write_pos_];
    lookahead_buffer_[lookahead_write_pos_] = input;
    lookahead_write_pos_ = (lookahead_write_pos_ + 1) % static_cast<int>(lookahead_buffer_.size());

    // True-peak detector on oversampled predictor.
    float instantTruePeak = 0.0f;
    for (int k = 0; k < oversample_factor_; ++k) {
        const float t = (static_cast<float>(k) + 1.0f) / static_cast<float>(oversample_factor_);
        const float x = prev_input_ + t * (input - prev_input_);
        instantTruePeak = std::max(instantTruePeak, std::fabs(x));
    }
    prev_input_ = input;

    if (instantTruePeak > true_peak_env_) {
        true_peak_env_ = instantTruePeak;
    } else {
        true_peak_env_ = detector_release_coeff_ * true_peak_env_ + (1.0f - detector_release_coeff_) * instantTruePeak;
    }

    // Adaptive gain computer: threshold stage into -1 dBTP ceiling.
    float targetGain = 1.0f;
    if (true_peak_env_ > threshold_) {
        targetGain = threshold_ / std::max(true_peak_env_, kEps);
    }
    if (true_peak_env_ > output_ceiling_) {
        const float ceilingGain = output_ceiling_ / std::max(true_peak_env_, kEps);
        targetGain = std::min(targetGain, ceilingGain);
    }

    // Adaptive release: deeper GR -> slower release.
    const float grDepthDb = std::max(0.0f, -20.0f * std::log10(std::max(targetGain, 1.0e-6f)));
    const float depthNorm = std::clamp(grDepthDb / 8.0f, 0.0f, 1.0f);
    const float relFast = release_coeff_fast_ + (release_coeff_slow_ - release_coeff_fast_) * depthNorm;
    const float relSlow = release_coeff_slow_;

    // Multi-stage release smoothing.
    if (targetGain < gain_stage1_) {
        gain_stage1_ = attack_coeff_ * gain_stage1_ + (1.0f - attack_coeff_) * targetGain;
    } else {
        gain_stage1_ = relFast * gain_stage1_ + (1.0f - relFast) * targetGain;
    }

    if (gain_stage1_ < gain_stage2_) {
        gain_stage2_ = attack_coeff_ * gain_stage2_ + (1.0f - attack_coeff_) * gain_stage1_;
    } else {
        gain_stage2_ = relSlow * gain_stage2_ + (1.0f - relSlow) * gain_stage1_;
    }

    // Stereo linking / M-S approximation (mono-safe): enforce extra protection on side-like content.
    link_lp_state_ += link_lp_alpha_ * (delayedInput - link_lp_state_);
    const float sideLike = std::fabs(delayedInput - link_lp_state_);
    const float linkedPeak = std::max(std::fabs(delayedInput), 0.7f * sideLike);
    if (linkedPeak * gain_stage2_ > output_ceiling_) {
        gain_stage2_ *= output_ceiling_ / std::max(linkedPeak * gain_stage2_, kEps);
    }

    // Oversampled program path + downsampling.
    float accum = 0.0f;
    for (int k = 0; k < oversample_factor_; ++k) {
        const float t = (static_cast<float>(k) + 1.0f) / static_cast<float>(oversample_factor_);
        float x = prev_delayed_input_ + t * (delayedInput - prev_delayed_input_);
        x *= gain_stage2_;

        if (soft_clip_enabled_) {
            // Gentle clip before ceiling clamp.
            x = std::tanh(soft_clip_drive_ * x) / std::tanh(soft_clip_drive_);
        }

        x = std::clamp(x, -output_ceiling_, output_ceiling_);
        accum += x;
    }
    prev_delayed_input_ = delayedInput;

    const float output = accum / static_cast<float>(oversample_factor_);

    // Meter uses post-smoothing gain reduction.
    if (gain_stage2_ < 1.0f) {
        gain_reduction_db_.store(20.0f * std::log10(std::max(gain_stage2_, 1.0e-6f)));
    } else {
        gain_reduction_db_.store(0.0f);
    }

    return output;
}

float Limiter::getGainReductionDB() const {
    return gain_reduction_db_.load();
}