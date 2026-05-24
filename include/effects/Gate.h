#pragma once
#include "effects/AudioEffect.h"
#include <cmath>
#include <algorithm>

/**
 * @brief Noise gate effect.
 * @details Attenuates signals below a set threshold to reduce noise.
 */
class Gate : public AudioEffect {
public:
    /** @brief Construct a Gate effect. */
    Gate();
    /** @brief Set the sample rate for the effect. */
    void setSampleRate(float rate) override;
    /** @brief Process a single audio sample. */
    float process(float input) override;
    /** @brief Get the effect name. */
    std::string name() const override { return "Gate"; }

    /** @brief Set the threshold in dB (e.g., -60.0). */
    void setThreshold(float threshold_dB);
    /** @brief Set the release time in milliseconds. */
    void setRelease(float release_ms);

private:
    void updateCoefficients();
    float msToCoeff(float ms) const;

    float sample_rate_ = 48000.0f; ///< Current sample rate
    float threshold_db_ = -45.0f;
    float threshold_ = 0.0056f; ///< Linear threshold
    float hysteresis_db_ = 6.0f;
    float knee_db_ = 6.0f;

    // Timing defaults requested: attack 1-5ms, hold 50-150ms, release 80-300ms.
    float attack_ms_ = 2.5f;
    float hold_ms_ = 90.0f;
    float release_ms_ = 140.0f;
    int hold_counter_samples_ = 0;
    int hold_samples_ = 0;

    // Sidechain HPF (~80-150 Hz target).
    float sidechain_hpf_hz_ = 110.0f;
    float hpf_a_ = 0.0f;
    float hpf_x1_ = 0.0f;
    float hpf_y1_ = 0.0f;

    // Detector: RMS + peak blend.
    float peak_env_ = 0.0f;
    float rms_env_sq_ = 0.0f;
    float detector_blend_ = 0.55f; // 0=RMS, 1=Peak
    float detector_attack_coeff_ = 0.0f;
    float detector_release_coeff_ = 0.0f;
    float rms_coeff_ = 0.0f;

    // Gate gain envelope and soft curve.
    float gain_env_ = 0.0f;
    float attack_coeff_ = 0.0f;
    float release_coeff_ = 0.0f;

    // Mono-safe stereo linking approximation (max of two detector estimates).
    float link_lp_state_ = 0.0f;
    float link_lp_alpha_ = 0.0f;

    bool gate_open_ = false;
};