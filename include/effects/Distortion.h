#pragma once
#include "effects/AudioEffect.h"
#include <algorithm>

/**
 * @brief Simple distortion effect.
 * @details Applies soft clipping and gain to the input signal.
 */
class Distortion : public AudioEffect {
public:
    /** @brief Construct a Distortion effect. */
    Distortion() = default;
    /** @brief Set the sample rate for the effect. */
    void setSampleRate(float rate) override;
    /** @brief Process a single audio sample. */
    float process(float input) override;
    /** @brief Get the effect name. */
    std::string name() const override { return "Distortion"; }

    /** @brief Set the drive amount (0.0 to 1.0). */
    void setDrive(float drive);
    /** @brief Set the wet/dry mix (0.0 = dry, 1.0 = wet). */
    void setMix(float mix);

private:
    float shapedCore(float x) const;
    void updateCoefficients();

    float drive_ = 0.5f; ///< Drive amount
    float mix_ = 1.0f; ///< Wet/dry mix

    float sample_rate_ = 48000.0f;
    bool oversampling_enabled_ = true;
    bool multiband_enabled_ = true;

    // Pre-filter HP/LP shaping.
    float pre_hp_lp_state_ = 0.0f;
    float pre_lp_state_ = 0.0f;
    float pre_hp_alpha_ = 0.0f;
    float pre_lp_alpha_ = 0.0f;

    // Oversampling and anti-imaging cleanup.
    float os_prev_input_ = 0.0f;
    float os_cleanup_lp_state_ = 0.0f;
    float os_cleanup_alpha_ = 0.0f;

    // Tone shaping post nonlinearity.
    float tone_lp_state_ = 0.0f;
    float tone_lp_alpha_ = 0.0f;

    // Optional multiband split states.
    float mb_low_state_ = 0.0f;
    float mb_mid_lp_state_ = 0.0f;
    float mb_low_alpha_ = 0.0f;
    float mb_mid_alpha_ = 0.0f;

    // Mono-safe stereo processing approximation.
    float side_lp_state_ = 0.0f;
    float side_lp_alpha_ = 0.0f;
    float stereo_width_ = 0.4f;
    float stereo_foldback_ = 0.2f;
};