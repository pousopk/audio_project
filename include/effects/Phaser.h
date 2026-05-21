#pragma once
#include "effects/AudioEffect.h"
#include <vector>

/**
 * @brief Phaser effect.
 * @details Modulates the input signal with a series of all-pass filters for a sweeping phase effect.
 */
class Phaser : public AudioEffect {
private:
    /**
     * @brief All-pass filter stage for Phaser.
     */
    class AllPassFilter {
    public:
        AllPassFilter();
        float process(float input);
        void setCoefficient(float coeff);
    private:
        float coefficient_ = 0.5f;
        float z1_ = 0.0f;
    };

public:
    /** @brief Construct a Phaser effect. */
    Phaser();
    /** @brief Set the sample rate for the effect. */
    void setSampleRate(float rate) override;
    /** @brief Process a single audio sample. */
    float process(float input) override;
    /** @brief Get the effect name. */
    std::string name() const override { return "Phaser"; }

    /** @brief Set the LFO rate in Hz. */
    void setRate(float rate_hz);
    /** @brief Set the modulation depth (0.0 to 1.0). */
    void setDepth(float depth);
    /** @brief Set the feedback amount (0.0 to < 1.0). */
    void setFeedback(float feedback);
    /** @brief Set the wet/dry mix. */
    void setMix(float mix);

private:
    float sample_rate_ = 48000.0f; ///< Current sample rate
    float rate_hz_ = 0.5f; ///< LFO rate
    float depth_ = 0.8f; ///< Modulation depth
    float feedback_ = 0.7f; ///< Feedback amount
    float mix_ = 0.5f; ///< Wet/dry mix
    float lfo_phase_ = 0.0f;
    float last_out_ = 0.0f;
    std::vector<AllPassFilter> filters_;
};