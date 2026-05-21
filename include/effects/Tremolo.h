#pragma once
#include "effects/AudioEffect.h"

/**
 * @brief Tremolo effect.
 * @details Modulates the amplitude of the input signal with a low-frequency oscillator.
 */
class Tremolo : public AudioEffect {
public:
    /** @brief Construct a Tremolo effect. */
    Tremolo();
    /** @brief Set the sample rate for the effect. */
    void setSampleRate(float rate) override;
    /** @brief Process a single audio sample. */
    float process(float input) override;
    /** @brief Get the effect name. */
    std::string name() const override { return "Tremolo"; }

    /** @brief Set the LFO rate in Hz. */
    void setRate(float rate_hz);
    /** @brief Set the modulation depth (0.0 to 1.0). */
    void setDepth(float depth);

private:
    float sample_rate_ = 48000.0f; ///< Current sample rate
    float rate_hz_ = 5.0f; ///< LFO rate
    float depth_ = 0.5f; ///< Modulation depth
    float mix_ = 1.0f; ///< Wet/dry mix (usually 1.0 for tremolo)
    float lfo_phase_ = 0.0f;
};