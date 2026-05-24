#pragma once
#include "effects/AudioEffect.h"
#include <atomic>
#include <cmath>
#include <algorithm>

/**
 * @brief Dynamic range compressor effect.
 * @details Reduces the dynamic range of the input signal above a threshold.
 */
class Compressor : public AudioEffect {
public:
    /** @brief Construct a Compressor effect. */
    Compressor();
    /** @brief Set the sample rate for the effect. */
    void setSampleRate(float rate) override;
    /** @brief Process a single audio sample. */
    float process(float input) override;
    /** @brief Get the effect name. */
    std::string name() const override { return "Compressor"; }

    /** @brief Set the threshold in dB (e.g., -20.0). */
    void setThreshold(float threshold_dB);
    /** @brief Set the compression ratio (e.g., 4.0 for 4:1). */
    void setRatio(float ratio);
    /** @brief Set the attack time in milliseconds. */
    void setAttack(float attack_ms);
    /** @brief Set the release time in milliseconds. */
    void setRelease(float release_ms);
    /** @brief Set soft knee width in dB. */
    void setKnee(float knee_db);
    /** @brief Set post-compression saturation drive. */
    void setSaturationDrive(float drive);
    /** @brief Set detector blend (0 = RMS, 1 = Peak). */
    void setDetectorBlend(float blend);

    /** @brief Get the current gain reduction in dB. */
    float getGainReductionDB() const;

private:
    static constexpr int kOversampling = 2;

    void updateCoefficients();
    float timeMsToCoeff(float ms, float sr) const;
    float linearToDb(float x) const;
    float dbToLinear(float db) const;
    float softKneeGainReductionDb(float detectorDb) const;

    float sample_rate_ = 48000.0f; ///< Current sample rate
    float threshold_db_ = -18.0f;
    float ratio_ = 4.0f;
    float attack_ms_ = 10.0f;
    float release_ms_ = 150.0f;

    // Sidechain filter (high-pass)
    float sidechain_hp_hz_ = 120.0f;
    float hp_alpha_ = 0.0f;
    float hp_x1_ = 0.0f;
    float hp_y1_ = 0.0f;

    // Hybrid peak/RMS detector
    float peak_env_ = 0.0f;
    float rms_state_ = 0.0f;
    float hybrid_blend_ = 0.6f; // 0=RMS, 1=Peak

    // Program-dependent timing and gain computer
    float detector_env_ = 0.0f;
    float knee_db_ = 6.0f;

    // Smoothed gain reduction and linking
    float gr_smooth_db_ = 0.0f;
    float gr_smooth_coeff_ = 0.0f;
    float linked_gr_db_ = 0.0f;
    float stereo_link_ = 1.0f; // Mono-safe, ready for stereo extension

    // Optional saturation
    bool saturation_enabled_ = true;
    float saturation_drive_ = 1.15f;

    float prev_input_ = 0.0f;

    std::atomic<float> gain_reduction_db_{0.0f}; ///< Gain reduction
};