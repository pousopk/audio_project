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
    float rate_hz_ = 4.0f; ///< LFO rate
    float depth_ = 0.35f; ///< Modulation depth
    float mix_ = 1.0f; ///< Wet/dry mix (usually 1.0 for tremolo)
    float lfo_phase_ = 0.0f;

    // Optional band split for psychoacoustic weighting.
    bool band_split_enabled_ = true;
    float split_alpha_ = 0.0f;
    float split_state_ = 0.0f;

    // Drifted modulation engine.
    float drift_phase_ = 0.0f;
    float drift_amount_ = 0.03f;
    float drift_rate_hz_ = 0.06f;

    // Stereo movement matrix approximation collapsed mono-safely.
    float stereo_phase_offset_rad_ = 2.3f; // ~132 deg

    // Optional saturation post modulation.
    bool saturation_enabled_ = true;
    float saturation_drive_ = 1.06f;
};