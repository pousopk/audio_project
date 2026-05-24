#pragma once
#include "effects/AudioEffect.h"
#include <vector>

/**
 * @brief Chorus effect.
 * @details Modulates the input signal with a delayed, modulated copy for a thickening effect.
 */
class Chorus : public AudioEffect {
public:
    /** @brief Construct a Chorus effect. */
    Chorus();
    /** @brief Set the sample rate for the effect. */
    void setSampleRate(float rate) override;
    /** @brief Process a single audio sample. */
    float process(float input) override;
    /** @brief Get the effect name. */
    std::string name() const override { return "Chorus"; }

    /** @brief Set the LFO rate in Hz. */
    void setRate(float rate_hz);
    /** @brief Set the modulation depth in milliseconds. */
    void setDepth(float depth_ms);
    /** @brief Set the wet/dry mix (0.0 = dry, 1.0 = wet). */
    void setMix(float mix);

private:
    struct VoiceState {
        float phaseOffset = 0.0f;
        float delayBiasMs = 0.0f;
        float filterState = 0.0f;
    };

    float sample_rate_ = 48000.0f; ///< Current sample rate
    float rate_hz_ = 0.5f; ///< LFO rate
    float depth_ms_ = 2.5f; ///< Modulation depth
    float mix_ = 0.5f; ///< Wet/dry mix
    float base_delay_ms_ = 12.0f;
    float feedback_ = 0.12f;
    float saturation_drive_ = 1.2f;

    static constexpr int kNumVoices = 6;

    std::vector<float> delay_buffer_;
    std::vector<VoiceState> voices_;
    int write_pos_ = 0;
    float lfo_phase_ = 0.0f;
    float pre_filter_state_ = 0.0f;
    float feedback_state_ = 0.0f;

    void updateLFO();
    float readDelayLinear(float delaySamples) const;
    float onePoleAlpha(float cutoffHz) const;
};