#/**
# * @file FXChain.cpp
# * @brief Implements the FXChain class for managing and processing a chain of audio effects.
# */

#include "effects/FXChain.h"
#include <algorithm>
#include "effects/Delay.h"
#include "effects/Reverb.h"
#include "effects/Distortion.h"
#include "effects/Compressor.h"
#include "effects/EQ.h"
#include "effects/Limiter.h"
#include "effects/Chorus.h"
#include "effects/Gate.h"
#include "effects/Flanger.h"
#include "effects/Phaser.h"
#include "effects/Tremolo.h"

namespace {
std::string effectName(const AudioEffect* effect) {
    if (dynamic_cast<const Delay*>(effect)) return "Delay";
    if (dynamic_cast<const Reverb*>(effect)) return "Reverb";
    if (dynamic_cast<const Distortion*>(effect)) return "Distortion";
    if (dynamic_cast<const Compressor*>(effect)) return "Compressor";
    if (dynamic_cast<const EQ*>(effect)) return "EQ";
    if (dynamic_cast<const Limiter*>(effect)) return "Limiter";
    if (dynamic_cast<const Chorus*>(effect)) return "Chorus";
    if (dynamic_cast<const Gate*>(effect)) return "Gate";
    if (dynamic_cast<const Flanger*>(effect)) return "Flanger";
    if (dynamic_cast<const Phaser*>(effect)) return "Phaser";
    if (dynamic_cast<const Tremolo*>(effect)) return "Tremolo";
    return "";
}

std::unique_ptr<AudioEffect> createEffectByName(const std::string& name) {
    if (name == "Delay") return std::make_unique<Delay>();
    if (name == "Reverb") return std::make_unique<Reverb>();
    if (name == "Distortion") return std::make_unique<Distortion>();
    if (name == "Compressor") return std::make_unique<Compressor>();
    if (name == "EQ") return std::make_unique<EQ>();
    if (name == "Limiter") return std::make_unique<Limiter>();
    if (name == "Chorus") return std::make_unique<Chorus>();
    if (name == "Gate") return std::make_unique<Gate>();
    if (name == "Flanger") return std::make_unique<Flanger>();
    if (name == "Phaser") return std::make_unique<Phaser>();
    if (name == "Tremolo") return std::make_unique<Tremolo>();
    return nullptr;
}
} // namespace

FXChain::FXChain() {
    // Now starts empty. Effects are created in setOrder.
}

void FXChain::setEnabled(bool enabled) {
    enabled_ = enabled;
}

void FXChain::setSampleRate(float rate) {
    sampleRate_ = rate;
    for (auto& effect : effects_) {
        effect->setSampleRate(rate);
    }
}

void FXChain::process(float* buffer, int nFrames) {
    if (!enabled_) {
        return;
    }

    for (int i = 0; i < nFrames; ++i) {
        if (effects_.empty()) {
            continue;
        }
        float sample = buffer[i];
        for (auto& effect : effects_) {
            sample = effect->process(sample);
        }
        buffer[i] = sample;
    }
}

void FXChain::setOrder(const std::vector<std::string>& newOrder) {
    std::vector<std::unique_ptr<AudioEffect>> oldEffects;
    oldEffects.swap(effects_);

    for (const auto& name : newOrder) {
        bool reused = false;
        for (auto& existing : oldEffects) {
            if (existing && effectName(existing.get()) == name) {
                effects_.push_back(std::move(existing));
                reused = true;
                break;
            }
        }

        if (!reused) {
            auto created = createEffectByName(name);
            if (created) {
                effects_.push_back(std::move(created));
            }
        }

        if (!effects_.empty()) {
            effects_.back()->setSampleRate(sampleRate_);
        }
    }
}

void FXChain::setReverbMix(float mix) {
    for (auto& effect : effects_) {
        if (auto* reverb = dynamic_cast<Reverb*>(effect.get())) {
            reverb->setMix(mix);
            return;
        }
    }
}

void FXChain::setTremoloRate(float rate_hz) {
    for (auto& effect : effects_) {
        if (auto* tremolo = dynamic_cast<Tremolo*>(effect.get())) {
            tremolo->setRate(rate_hz);
            return;
        }
    }
}

void FXChain::setTremoloDepth(float depth) {
    for (auto& effect : effects_) {
        if (auto* tremolo = dynamic_cast<Tremolo*>(effect.get())) {
            tremolo->setDepth(depth);
            return;
        }
    }
}

void FXChain::setPhaserRate(float rate_hz) {
    for (auto& effect : effects_) {
        if (auto* phaser = dynamic_cast<Phaser*>(effect.get())) {
            phaser->setRate(rate_hz);
            return;
        }
    }
}

void FXChain::setPhaserDepth(float depth) {
    for (auto& effect : effects_) {
        if (auto* phaser = dynamic_cast<Phaser*>(effect.get())) {
            phaser->setDepth(depth);
            return;
        }
    }
}

void FXChain::setPhaserFeedback(float feedback) {
    for (auto& effect : effects_) {
        if (auto* phaser = dynamic_cast<Phaser*>(effect.get())) {
            phaser->setFeedback(feedback);
            return;
        }
    }
}

void FXChain::setPhaserMix(float mix) {
    for (auto& effect : effects_) {
        if (auto* phaser = dynamic_cast<Phaser*>(effect.get())) {
            phaser->setMix(mix);
            return;
        }
    }
}

void FXChain::setFlangerRate(float rate_hz) {
    for (auto& effect : effects_) {
        if (auto* flanger = dynamic_cast<Flanger*>(effect.get())) {
            flanger->setRate(rate_hz);
            return;
        }
    }
}

void FXChain::setFlangerDepth(float depth_ms) {
    for (auto& effect : effects_) {
        if (auto* flanger = dynamic_cast<Flanger*>(effect.get())) {
            flanger->setDepth(depth_ms);
            return;
        }
    }
}

void FXChain::setFlangerFeedback(float feedback) {
    for (auto& effect : effects_) {
        if (auto* flanger = dynamic_cast<Flanger*>(effect.get())) {
            flanger->setFeedback(feedback);
            return;
        }
    }
}

void FXChain::setFlangerMix(float mix) {
    for (auto& effect : effects_) {
        if (auto* flanger = dynamic_cast<Flanger*>(effect.get())) {
            flanger->setMix(mix);
            return;
        }
    }
}

void FXChain::setGateThreshold(float threshold_db) {
    for (auto& effect : effects_) {
        if (auto* gate = dynamic_cast<Gate*>(effect.get())) {
            gate->setThreshold(threshold_db);
            return;
        }
    }
}

void FXChain::setGateRelease(float release_ms) {
    for (auto& effect : effects_) {
        if (auto* gate = dynamic_cast<Gate*>(effect.get())) {
            gate->setRelease(release_ms);
            return;
        }
    }
}

void FXChain::setChorusRate(float rate_hz) {
    for (auto& effect : effects_) {
        if (auto* chorus = dynamic_cast<Chorus*>(effect.get())) {
            chorus->setRate(rate_hz);
            return;
        }
    }
}

void FXChain::setChorusDepth(float depth_ms) {
    for (auto& effect : effects_) {
        if (auto* chorus = dynamic_cast<Chorus*>(effect.get())) {
            chorus->setDepth(depth_ms);
            return;
        }
    }
}

void FXChain::setChorusMix(float mix) {
    for (auto& effect : effects_) {
        if (auto* chorus = dynamic_cast<Chorus*>(effect.get())) {
            chorus->setMix(mix);
            return;
        }
    }
}

void FXChain::setLimiterThreshold(float threshold_db) {
    for (auto& effect : effects_) {
        if (auto* limiter = dynamic_cast<Limiter*>(effect.get())) {
            limiter->setThreshold(threshold_db);
            return;
        }
    }
}

void FXChain::setLimiterRelease(float release_ms) {
    for (auto& effect : effects_) {
        if (auto* limiter = dynamic_cast<Limiter*>(effect.get())) {
            limiter->setRelease(release_ms);
            return;
        }
    }
}


void FXChain::setEQLowGain(float gain_db) {
    for (auto& effect : effects_) {
        if (auto* eq = dynamic_cast<EQ*>(effect.get())) {
            eq->setLowGain(gain_db);
            return;
        }
    }
}

void FXChain::setEQMidGain(float gain_db) {
    for (auto& effect : effects_) {
        if (auto* eq = dynamic_cast<EQ*>(effect.get())) {
            eq->setMidGain(gain_db);
            return;
        }
    }
}

void FXChain::setEQHighGain(float gain_db) {
    for (auto& effect : effects_) {
        if (auto* eq = dynamic_cast<EQ*>(effect.get())) {
            eq->setHighGain(gain_db);
            return;
        }
    }
}

void FXChain::setEQSaturationEnabled(bool enabled) {
    for (auto& effect : effects_) {
        if (auto* eq = dynamic_cast<EQ*>(effect.get())) {
            eq->setSaturationEnabled(enabled);
            return;
        }
    }
}

void FXChain::setEQSaturationDrive(float drive) {
    for (auto& effect : effects_) {
        if (auto* eq = dynamic_cast<EQ*>(effect.get())) {
            eq->setSaturationDrive(drive);
            return;
        }
    }
}

void FXChain::setEQMSAmount(float amount) {
    for (auto& effect : effects_) {
        if (auto* eq = dynamic_cast<EQ*>(effect.get())) {
            eq->setMSAmount(amount);
            return;
        }
    }
}

void FXChain::setCompressorThreshold(float threshold_dB) {
    for (auto& effect : effects_) {
        if (auto* comp = dynamic_cast<Compressor*>(effect.get())) {
            comp->setThreshold(threshold_dB);
            return;
        }
    }
}

void FXChain::setCompressorRatio(float ratio) {
    for (auto& effect : effects_) {
        if (auto* comp = dynamic_cast<Compressor*>(effect.get())) {
            comp->setRatio(ratio);
            return;
        }
    }
}

void FXChain::setCompressorAttack(float attack_ms) {
    for (auto& effect : effects_) {
        if (auto* comp = dynamic_cast<Compressor*>(effect.get())) {
            comp->setAttack(attack_ms);
            return;
        }
    }
}

void FXChain::setCompressorRelease(float release_ms) {
    for (auto& effect : effects_) {
        if (auto* comp = dynamic_cast<Compressor*>(effect.get())) {
            comp->setRelease(release_ms);
            return;
        }
    }
}

void FXChain::setCompressorKnee(float knee_db) {
    for (auto& effect : effects_) {
        if (auto* comp = dynamic_cast<Compressor*>(effect.get())) {
            comp->setKnee(knee_db);
            return;
        }
    }
}

void FXChain::setCompressorSaturationDrive(float drive) {
    for (auto& effect : effects_) {
        if (auto* comp = dynamic_cast<Compressor*>(effect.get())) {
            comp->setSaturationDrive(drive);
            return;
        }
    }
}

void FXChain::setCompressorDetectorBlend(float blend) {
    for (auto& effect : effects_) {
        if (auto* comp = dynamic_cast<Compressor*>(effect.get())) {
            comp->setDetectorBlend(blend);
            return;
        }
    }
}

void FXChain::setDelayTime(float time_ms) {
    for (auto& effect : effects_) {
        if (auto* delay = dynamic_cast<Delay*>(effect.get())) {
            delay->setDelayTime(time_ms);
            return;
        }
    }
}

void FXChain::setDelayFeedback(float feedback) {
    for (auto& effect : effects_) {
        if (auto* delay = dynamic_cast<Delay*>(effect.get())) {
            delay->setFeedback(feedback);
            return;
        }
    }
}

void FXChain::setDelayMix(float mix) {
    for (auto& effect : effects_) {
        if (auto* delay = dynamic_cast<Delay*>(effect.get())) {
            delay->setMix(mix);
            return;
        }
    }
}

void FXChain::setDistortionDrive(float drive) {
    for (auto& effect : effects_) {
        if (auto* distortion = dynamic_cast<Distortion*>(effect.get())) {
            distortion->setDrive(drive);
            return;
        }
    }
}

void FXChain::setDistortionMix(float mix) {
    for (auto& effect : effects_) {
        if (auto* distortion = dynamic_cast<Distortion*>(effect.get())) {
            distortion->setMix(mix);
            return;
        }
    }
}

float FXChain::getCompressorGainReductionDB() const {
    for (const auto& effect : effects_) {
        if (const auto* comp = dynamic_cast<const Compressor*>(effect.get())) {
            return comp->getGainReductionDB();
        }
    }
    return 0.0f; // No compressor in chain
}