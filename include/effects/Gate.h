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

    float sample_rate_ = 48000.0f; ///< Current sample rate
    float threshold_ = 0.001f; ///< Linear threshold
    float release_ms_ = 100.0f; ///< Release time
    float attack_coeff_ = 0.999f; ///< Fast attack
    float release_coeff_ = 0.999f;
    float envelope_ = 0.0f;
    bool gate_open_ = false;
};