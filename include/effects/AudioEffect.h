#pragma once
#include <vector>
#include <string>

class AudioEffect {
public:
    virtual ~AudioEffect() = default;
    virtual void setSampleRate(float rate) = 0;
    virtual float process(float input) = 0;
    virtual void process(float* buffer, int numSamples) {
        for (int i = 0; i < numSamples; ++i) {
            buffer[i] = process(buffer[i]);
        }
    }
    virtual std::string name() const = 0;
};
