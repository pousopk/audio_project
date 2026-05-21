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

    /** @brief Get the current gain reduction in dB. */
    float getGainReductionDB() const;

private:
    void updateCoefficients();

    float sample_rate_ = 48000.0f; ///< Current sample rate
    float threshold_ = 1.0f; ///< Linear threshold
    float ratio_ = 1.0f; ///< Compression ratio
    float attack_ms_ = 5.0f; ///< Attack time
    float release_ms_ = 100.0f; ///< Release time
    float attack_coeff_ = 0.999f;
    float release_coeff_ = 0.999f;
    float envelope_ = 0.0f;
    std::atomic<float> gain_reduction_db_{0.0f}; ///< Gain reduction
};