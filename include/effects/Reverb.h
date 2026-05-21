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
    /**
     * @brief Comb filter section for reverb.
     */
    class Comb {
    public:
        void set(float feedback, float damp);
        float process(float input);
        std::vector<float> buffer;
        float feedback_ = 0.84f;
        float filter_store_ = 0.0f;
        float damp_ = 0.2f;
        int buf_size_ = 0;
        int buf_idx_ = 0;
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
        int buf_size_ = 0;
        int buf_idx_ = 0;
    };

    std::vector<Comb> combs_;
    std::vector<AllPass> allpasses_;
    float mix_ = 0.3f; ///< Wet/dry mix
};