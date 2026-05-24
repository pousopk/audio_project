#pragma once
#include "effects/AudioEffect.h"
#include <atomic>
#include <vector>
#include <cmath>
#include <algorithm>

/**
 * @brief Brickwall limiter effect.
 * @details Prevents the signal from exceeding a set threshold.
 */
class Limiter : public AudioEffect {
public:
    /** @brief Construct a Limiter effect. */
    Limiter();
    /** @brief Set the sample rate for the effect. */
    void setSampleRate(float rate) override;
    /** @brief Process a single audio sample. */
    float process(float input) override;
    /** @brief Get the effect name. */
    std::string name() const override { return "Limiter"; }

    /** @brief Set the threshold in dB (e.g., -0.1). */
    void setThreshold(float threshold_dB);
    /** @brief Set the release time in milliseconds. */
    void setRelease(float release_ms);

    /** @brief Get the current gain reduction in dB. */
    float getGainReductionDB() const;

private:
    void updateCoefficients();
    float msToCoeff(float ms) const;

    float sample_rate_ = 48000.0f; ///< Current sample rate
    float threshold_db_ = -1.0f;
    float threshold_ = 0.8912509f;
    float release_ms_ = 120.0f;

    // Requested architecture defaults.
    int oversample_factor_ = 4; // 4x-8x range, default 4x
    float lookahead_ms_ = 2.0f; // 1-3 ms target
    float output_ceiling_dbtp_ = -1.0f;
    float output_ceiling_ = 0.8912509f;

    std::vector<float> lookahead_buffer_;
    int lookahead_write_pos_ = 0;
    int lookahead_samples_ = 1;

    float prev_input_ = 0.0f;
    float prev_delayed_input_ = 0.0f;

    // True-peak detector state.
    float true_peak_env_ = 0.0f;
    float detector_release_coeff_ = 0.0f;

    // Adaptive gain computer and multi-stage smoothing.
    float gain_stage1_ = 1.0f;
    float gain_stage2_ = 1.0f;
    float attack_coeff_ = 0.0f;
    float release_coeff_fast_ = 0.0f;
    float release_coeff_slow_ = 0.0f;

    // Optional soft clipper.
    bool soft_clip_enabled_ = true;
    float soft_clip_drive_ = 1.18f;

    // Mono-safe stereo-link / M-S approximation.
    float link_lp_state_ = 0.0f;
    float link_lp_alpha_ = 0.0f;

    std::atomic<float> gain_reduction_db_{0.0f}; ///< Gain reduction
};