#pragma once
#include <vector>
#include "effects/AudioEffect.h"

/**
 * @brief Simple delay effect with feedback and mix control.
 * @details Implements a basic delay line with adjustable delay time, feedback, and wet/dry mix.
 */
class Delay : public AudioEffect {
public:
    /** @brief Construct a Delay effect. */
    Delay();
    /** @brief Set the sample rate for the effect. */
    void setSampleRate(float rate) override;
    /** @brief Set the delay time in milliseconds. */
    void setDelayTime(float time_ms);
    /** @brief Set the feedback amount (0.0 to < 1.0). */
    void setFeedback(float feedback);
    /** @brief Set the wet/dry mix (0.0 = dry, 1.0 = wet). */
    void setMix(float mix);
    /** @brief Process a single audio sample. */
    float process(float input) override;
    /** @brief Get the effect name. */
    std::string name() const override { return "Delay"; }

private:
    std::vector<float> buffer_; ///< Delay buffer
    int write_pos_ = 0; ///< Write position in buffer
    int delay_samples_ = 0; ///< Delay length in samples
    float sample_rate_ = 48000.0f; ///< Current sample rate
    float feedback_ = 0.5f; ///< Feedback amount
    float mix_ = 0.3f; ///< Wet/dry mix
};