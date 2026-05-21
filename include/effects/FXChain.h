#pragma once
#include <vector>
#include <memory>
#include "effects/AudioEffect.h"

class FXChain {
public:
    FXChain();
    void addEffect(std::unique_ptr<AudioEffect> effect);
    void process(float* buffer, int numSamples);
    void setSampleRate(float rate);
    void setOrder(const std::vector<std::string>& newOrder);
    void setReverbMix(float mix);
    void setTremoloRate(float rate_hz);
    void setTremoloDepth(float depth);
    void setPhaserRate(float rate_hz);
    void setPhaserDepth(float depth);
    void setPhaserFeedback(float feedback);
    void setPhaserMix(float mix);
    void setFlangerRate(float rate_hz);
    void setFlangerDepth(float depth_ms);
    void setFlangerFeedback(float feedback);
    void setFlangerMix(float mix);
    void setGateThreshold(float threshold_db);
    void setGateRelease(float release_ms);
    void setChorusRate(float rate_hz);
    void setChorusDepth(float depth_ms);
    void setChorusMix(float mix);
    void setLimiterThreshold(float threshold_db);
    void setLimiterRelease(float release_ms);
    void setEQLowGain(float gain_db);
    void setEQMidGain(float gain_db);
    void setEQHighGain(float gain_db);
    void setCompressorThreshold(float threshold_dB);
    void setCompressorRatio(float ratio);
    void setCompressorAttack(float attack_ms);
    void setCompressorRelease(float release_ms);
    void setDelayTime(float time_ms);
    void setDelayFeedback(float feedback);
    void setDelayMix(float mix);
    void setDistortionDrive(float drive);
    void setDistortionMix(float mix);
    float getCompressorGainReductionDB() const;
private:
    std::vector<std::unique_ptr<AudioEffect>> effects_;
};
