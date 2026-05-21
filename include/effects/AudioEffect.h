#pragma once
#include <vector>
#include <string>

/**
 * @brief Abstract base class for audio effects.
 */
class AudioEffect {
public:
    /** @brief Virtual destructor. */
    virtual ~AudioEffect() = default;
    /** @brief Set the sample rate for the effect. */
    virtual void setSampleRate(float rate) = 0;
    /** @brief Process a single audio sample. */
    virtual float process(float input) = 0;
    /** @brief Process a buffer of audio samples. */
    virtual void process(float* buffer, int numSamples) {
        for (int i = 0; i < numSamples; ++i) {
            buffer[i] = process(buffer[i]);
        }
    }
    /** @brief Get the effect name. */
    virtual std::string name() const = 0;
};
