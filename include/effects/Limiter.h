#pragma once
#include "effects/AudioEffect.h"
#include <atomic>
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

    float sample_rate_ = 48000.0f; ///< Current sample rate
    float threshold_ = 1.0f; ///< Linear threshold
    float release_ms_ = 50.0f; ///< Release time
    float attack_coeff_ = 0.999f;
    float release_coeff_ = 0.999f;
    float envelope_ = 0.0f;
    float gain_ = 1.0f;
    std::atomic<float> gain_reduction_db_{0.0f}; ///< Gain reduction
};