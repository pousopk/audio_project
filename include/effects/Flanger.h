#pragma once
#include "effects/AudioEffect.h"
#include <vector>

/**
 * @brief Flanger effect.
 * @details Modulates the input signal with a short, variable delay for a sweeping effect.
 */
class Flanger : public AudioEffect {
public:
    /** @brief Construct a Flanger effect. */
    Flanger();
    /** @brief Set the sample rate for the effect. */
    void setSampleRate(float rate) override;
    /** @brief Process a single audio sample. */
    float process(float input) override;
    /** @brief Get the effect name. */
    std::string name() const override { return "Flanger"; }

    /** @brief Set the LFO rate in Hz. */
    void setRate(float rate_hz);
    /** @brief Set the modulation depth in milliseconds. */
    void setDepth(float depth_ms);
    /** @brief Set the feedback amount (0.0 to < 1.0). */
    void setFeedback(float feedback);
    /** @brief Set the wet/dry mix (0.0 = dry, 1.0 = wet). */
    void setMix(float mix);

private:
    float sample_rate_ = 48000.0f; ///< Current sample rate
    float rate_hz_ = 0.2f; ///< LFO rate
    float depth_ms_ = 1.5f; ///< Modulation depth
    float feedback_ = 0.5f; ///< Feedback amount
    float mix_ = 0.5f; ///< Wet/dry mix

    std::vector<float> delay_buffer_;
    int write_pos_ = 0;
    float lfo_phase_ = 0.0f;
};