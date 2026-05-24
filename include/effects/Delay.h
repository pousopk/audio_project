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
    float readDelayLinear(float delaySamples) const;
    float randomSigned();

    std::vector<float> buffer_;
    int write_pos_ = 0;
    float delay_time_ms_ = 500.0f;
    float delay_samples_ = 0.0f;
    float sample_rate_ = 48000.0f;
    float feedback_ = 0.5f;
    float mix_ = 0.3f;

    // Slow random modulation for wow/flutter style movement.
    float mod_depth_ms_ = 4.0f;
    float mod_current_ms_ = 0.0f;
    float mod_target_ms_ = 0.0f;
    int mod_counter_samples_ = 0;
    unsigned int rand_state_ = 0x9e3779b9u;

    // Feedback loop tone/saturation.
    float feedback_lpf_state_ = 0.0f;
    float feedback_lpf_alpha_ = 0.0f;
    float loop_saturation_drive_ = 1.15f;

    // Gentle diffusion all-pass stage on the wet path.
    std::vector<float> diffusion_buffer_;
    int diffusion_write_pos_ = 0;
    float diffusion_g_ = 0.35f;

    // Mono-safe stereo decorrelation approximation (dual taps).
    float decorrelation_offset_ms_ = 3.1f;
};