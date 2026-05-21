#pragma once
#include "effects/AudioEffect.h"

/**
 * @brief 3-band parametric equalizer effect.
 * @details Provides low, mid, and high frequency gain controls using biquad filters.
 */
class EQ : public AudioEffect {
public:
    /**
     * @brief Biquad filter section for EQ.
     */
    struct Biquad {
        float process(float input);
        void calculate_low_shelf(float sample_rate, float freq, float gain_db);
        void calculate_peaking(float sample_rate, float freq, float q, float gain_db);
        void calculate_high_shelf(float sample_rate, float freq, float gain_db);

        float a0 = 1.0f, a1 = 0.0f, a2 = 0.0f, b1 = 0.0f, b2 = 0.0f;
        float x1 = 0.0f, x2 = 0.0f, y1 = 0.0f, y2 = 0.0f;
    };

    /** @brief Construct an EQ effect. */
    EQ();
    /** @brief Set the sample rate for the effect. */
    void setSampleRate(float rate) override;
    /** @brief Process a single audio sample. */
    float process(float input) override;
    /** @brief Get the effect name. */
    std::string name() const override { return "EQ"; }

    /** @brief Set low band gain in dB (-15 to +15). */
    void setLowGain(float gain_db);
    /** @brief Set mid band gain in dB (-15 to +15). */
    void setMidGain(float gain_db);
    /** @brief Set high band gain in dB (-15 to +15). */
    void setHighGain(float gain_db);

private:
    void updateFilters();

    float sample_rate_ = 48000.0f; ///< Current sample rate
    float low_gain_db_ = 0.0f; ///< Low band gain
    float mid_gain_db_ = 0.0f; ///< Mid band gain
    float high_gain_db_ = 0.0f; ///< High band gain
    Biquad low_shelf_;
    Biquad mid_peak_;
    Biquad high_shelf_;
};