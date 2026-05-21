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
    void setSampleRate(float rate) override { (void)rate; }
    /** @brief Process a single audio sample. */
    float process(float input) override;
    /** @brief Get the effect name. */
    std::string name() const override { return "Distortion"; }

    /** @brief Set the drive amount (0.0 to 1.0). */
    void setDrive(float drive);
    /** @brief Set the wet/dry mix (0.0 = dry, 1.0 = wet). */
    void setMix(float mix);

private:
    float drive_ = 0.5f; ///< Drive amount
    float mix_ = 1.0f; ///< Wet/dry mix
};