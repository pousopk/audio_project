#pragma once
#include <vector>
#include "effects/AudioEffect.h"

/**
 * @brief Simple Schroeder reverb effect.
 * @details Implements a basic reverb using comb and all-pass filters.
 */
class Reverb : public AudioEffect {
public:
    /** @brief Construct a Reverb effect. */
    Reverb();
    /** @brief Set the sample rate for the effect. */
    void setSampleRate(float rate) override;
    /** @brief Set the wet/dry mix (0.0 = dry, 1.0 = wet). */
    void setMix(float mix);
    /** @brief Process a single audio sample. */
    float process(float input) override;
    /** @brief Get the effect name. */
    std::string name() const override { return "Reverb"; }

private:
    void initNetwork();
    float readFromBuffer(const std::vector<float>& buffer, int writePos, float delaySamples) const;

    /**
     * @brief Comb filter section for reverb.
     */
    class Comb {
    public:
        void set(float sampleRate, float feedback, float dampHz, float toneHz, float baseDelaySamples, float modRateHz, float modDepthSamples);
        float process(float input);
        float readLinear(float delaySamples) const;

        std::vector<float> buffer;
        float sample_rate_ = 48000.0f;
        float feedback_ = 0.80f;
        float damp_alpha_ = 0.0f;
        float tone_alpha_ = 0.0f;
        float damp_state_ = 0.0f;
        float tone_state_ = 0.0f;
        float base_delay_samples_ = 1000.0f;
        float mod_rate_hz_ = 0.1f;
        float mod_depth_samples_ = 2.5f;
        float mod_phase_ = 0.0f;
        float mod_target_ = 0.0f;
        float mod_current_ = 0.0f;
        int mod_hold_counter_ = 0;
        unsigned int rand_state_ = 0x12345678u;
        int write_pos_ = 0;
    };

    /**
     * @brief All-pass filter section for reverb.
     */
    class AllPass {
    public:
        void set(float feedback);
        float process(float input);
        std::vector<float> buffer;
        float feedback_ = 0.5f;
        int write_pos_ = 0;
    };

    std::vector<Comb> combs_;
    std::vector<AllPass> diffusers_a_;
    std::vector<AllPass> diffusers_b_;

    std::vector<float> predelay_buffer_;
    int predelay_write_pos_ = 0;
    float predelay_ms_ = 18.0f;
    float sample_rate_ = 48000.0f;

    std::vector<float> early_tap_ms_;
    float hf_damp_hz_ = 6800.0f;
    float tone_hz_ = 4200.0f;

    float mix_ = 0.3f; ///< Wet/dry mix
};