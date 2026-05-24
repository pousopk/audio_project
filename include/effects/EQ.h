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
    /** @brief Enable or disable post-EQ saturation. */
    void setSaturationEnabled(bool enabled);
    /** @brief Set post-EQ saturation drive. */
    void setSaturationDrive(float drive);
    /** @brief Set combined M/S-style amount (0..1). */
    void setMSAmount(float amount);

private:
    float processCore(float input);
    void updateSmoothingAndFilters();
    void updateFilters();

    float sample_rate_ = 48000.0f; ///< Current sample rate
    float low_gain_db_ = 0.0f; ///< Low band gain
    float mid_gain_db_ = 0.0f; ///< Mid band gain
    float high_gain_db_ = 0.0f; ///< High band gain
    float low_gain_db_smoothed_ = 0.0f;
    float mid_gain_db_smoothed_ = 0.0f;
    float high_gain_db_smoothed_ = 0.0f;
    float gain_smooth_coeff_ = 0.0015f;
    float mid_q_base_ = 1.0f;
    float mid_q_adapted_ = 1.0f;

    bool oversampling_enabled_ = true;
    float os_prev_input_ = 0.0f;

    bool saturation_enabled_ = true;
    float saturation_drive_ = 1.15f;

    // Mono-safe approximation of an M/S stage by folding a small side estimate.
    float ms_width_ = 0.35f;
    float ms_foldback_ = 0.2f;
    float ms_side_lp_state_ = 0.0f;
    float ms_side_lp_alpha_ = 0.04f;

    Biquad low_shelf_;
    Biquad mid_peak_;
    Biquad high_shelf_;
};