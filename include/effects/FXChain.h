#pragma once
#include <vector>
#include <memory>
#include "effects/AudioEffect.h"

/**
 * @brief Manages a chain of audio effects and their parameters.
 */
class FXChain {
public:
    /** @brief Construct an FXChain. */
    FXChain();
    /** @brief Add an effect to the chain. */
    void addEffect(std::unique_ptr<AudioEffect> effect);
    /** @brief Process a buffer through the effect chain. */
    void process(float* buffer, int numSamples);
    /** @brief Set the sample rate for all effects. */
    void setSampleRate(float rate);
    /** @brief Set the order of effects by name. */
    void setOrder(const std::vector<std::string>& newOrder);
    /** @brief Set reverb mix. */
    void setReverbMix(float mix);
    /** @brief Set tremolo rate. */
    void setTremoloRate(float rate_hz);
    /** @brief Set tremolo depth. */
    void setTremoloDepth(float depth);
    /** @brief Set phaser rate. */
    void setPhaserRate(float rate_hz);
    /** @brief Set phaser depth. */
    void setPhaserDepth(float depth);
    /** @brief Set phaser feedback. */
    void setPhaserFeedback(float feedback);
    /** @brief Set phaser mix. */
    void setPhaserMix(float mix);
    /** @brief Set flanger rate. */
    void setFlangerRate(float rate_hz);
    /** @brief Set flanger depth. */
    void setFlangerDepth(float depth_ms);
    /** @brief Set flanger feedback. */
    void setFlangerFeedback(float feedback);
    /** @brief Set flanger mix. */
    void setFlangerMix(float mix);
    /** @brief Set gate threshold. */
    void setGateThreshold(float threshold_db);
    /** @brief Set gate release. */
    void setGateRelease(float release_ms);
    /** @brief Set chorus rate. */
    void setChorusRate(float rate_hz);
    /** @brief Set chorus depth. */
    void setChorusDepth(float depth_ms);
    /** @brief Set chorus mix. */
    void setChorusMix(float mix);
    /** @brief Set limiter threshold. */
    void setLimiterThreshold(float threshold_db);
    /** @brief Set limiter release. */
    void setLimiterRelease(float release_ms);
    /** @brief Set EQ low gain. */
    void setEQLowGain(float gain_db);
    /** @brief Set EQ mid gain. */
    void setEQMidGain(float gain_db);
    /** @brief Set EQ high gain. */
    void setEQHighGain(float gain_db);
    /** @brief Set compressor threshold. */
    void setCompressorThreshold(float threshold_dB);
    /** @brief Set compressor ratio. */
    void setCompressorRatio(float ratio);
    /** @brief Set compressor attack time. */
    void setCompressorAttack(float attack_ms);
    /** @brief Set compressor release time. */
    void setCompressorRelease(float release_ms);
    /** @brief Set delay time. */
    void setDelayTime(float time_ms);
    /** @brief Set delay feedback. */
    void setDelayFeedback(float feedback);
    /** @brief Set delay mix. */
    void setDelayMix(float mix);
    /** @brief Set distortion drive. */
    void setDistortionDrive(float drive);
    /** @brief Set distortion mix. */
    void setDistortionMix(float mix);
    /** @brief Get compressor gain reduction in dB. */
    float getCompressorGainReductionDB() const;
private:
    std::vector<std::unique_ptr<AudioEffect>> effects_; ///< List of effect instances
};
