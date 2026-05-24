#include "synths/FMSynth.h"
#include <algorithm>
#include <array>
#include <cmath>

namespace {
constexpr std::array<int, 6> kStringMidi = {40, 45, 50, 55, 59, 64};

int midiForStringFret(int string, int fret) {
    return kStringMidi[string] + fret;
}

int nthLowestPitchedString(const std::vector<int>& shapeFrets, int rank) {
    std::vector<std::pair<int, int>> pitchedStrings;
    for (int s = 0; s < 6; ++s) {
        const int fret = static_cast<int>(shapeFrets.size()) > s ? shapeFrets[s] : -1;
        if (fret == -1) continue;
        pitchedStrings.emplace_back(midiForStringFret(s, fret), s);
    }
    if (pitchedStrings.empty()) return -1;

    std::sort(pitchedStrings.begin(), pitchedStrings.end(),
              [](const auto& a, const auto& b) {
                  if (a.first != b.first) return a.first < b.first;
                  return a.second < b.second;
              });

    rank = std::clamp(rank, 0, static_cast<int>(pitchedStrings.size()) - 1);
    return pitchedStrings[rank].second;
}
} // namespace

FMSynth::FMSynth() {}

void FMSynth::setSampleRate(unsigned int rate) {
    sampleRate_ = rate;
}

void FMSynth::setVolume(double volume) {
    volume_ = volume;
}

void FMSynth::setModRatio(double ratio) {
    const double clamped = std::clamp(ratio, 0.1, 12.0);
    for (int s = 0; s < kNumStrings; ++s) {
        voices_[s].modRatio = clamped;
    }
}

void FMSynth::setModIndex(double index) {
    const double clamped = std::clamp(index, 0.0, 20.0);
    for (int s = 0; s < kNumStrings; ++s) {
        voices_[s].modIndex = clamped;
    }
}

void FMSynth::setEnvelope(double attack, double decay, double sustain, double release) {
    attack_ = std::clamp(attack, 0.001, 2.0);
    decay_ = std::clamp(decay, 0.001, 2.0);
    sustain_ = std::clamp(sustain, 0.0, 1.0);
    release_ = std::clamp(release, 0.001, 4.0);
}

void FMSynth::noteOn(int string, double freq, int absSample) {
    if (string < 0 || string >= kNumStrings) return;
    voices_[string].carrierFreq = freq;
    voices_[string].envStage = 1;
    voices_[string].lastAttackSample = absSample;
}

double FMSynth::nextEnvelope(Voice& v, int absSample) {
    double dt = 1.0 / sampleRate_;
    int attackPos = absSample - v.lastAttackSample;
    const double releaseSamplesShort = 0.05 * sampleRate_;
    if (attackPos > releaseSamplesShort) {
        v.envStage = 4;
    }
    switch (v.envStage) {
        case 0: return 0.0;
        case 1:
            v.env += dt / attack_;
            if (v.env >= 1.0) { v.env = 1.0; v.envStage = 2; }
            break;
        case 2:
            v.env -= dt * (1.0 - sustain_) / decay_;
            if (v.env <= sustain_) { v.env = sustain_; v.envStage = 3; }
            break;
        case 3:
            break;
        case 4:
            v.env -= dt * sustain_ / release_;
            if (v.env <= 0.0) { v.env = 0.0; v.envStage = 0; }
            break;
    }
    return v.env;
}

double FMSynth::nextSample(Voice& v) {
    double modFreq = v.carrierFreq * v.modRatio;
    double modSignal = std::sin(2.0 * M_PI * v.modPhase) * v.modIndex;
    double carrier = std::sin(2.0 * M_PI * v.carrierPhase + modSignal);
    v.modPhase += modFreq / sampleRate_;
    v.carrierPhase += v.carrierFreq / sampleRate_;
    if (v.modPhase >= 1.0) v.modPhase -= 1.0;
    if (v.carrierPhase >= 1.0) v.carrierPhase -= 1.0;
    return carrier;
}

void FMSynth::process(float* out,
                     unsigned int nFrames,
                     const std::vector<int>& shapeFrets,
                     const StrummingPattern& strummingPattern,
                     int samplesPerMeasure,
                     int measureSampleCounter,
                     int beatsPerBar,
                     int samplesPerBeat) {
    std::vector<std::vector<int>> stringAttackSchedule(kNumStrings);
    for (const auto& ev : strummingPattern.events) {
        int evSample = static_cast<int>(ev.position * samplesPerMeasure);
        if (evSample < 0 || evSample >= samplesPerMeasure) continue;
        StrumType type = ev.type;
        int numNotes = ev.numNotes;
        std::vector<int> stringOrder;
        if (type == StrumType::Down || type == StrumType::ArpeggioAsc || type == StrumType::DownAltBass) {
            for (int s = 0; s < kNumStrings; ++s) if ((int)shapeFrets.size() > s && shapeFrets[s] != -1) stringOrder.push_back(s);
        } else if (type == StrumType::Up || type == StrumType::ArpeggioDesc) {
            for (int s = kNumStrings - 1; s >= 0; --s) if ((int)shapeFrets.size() > s && shapeFrets[s] != -1) stringOrder.push_back(s);
        }
        if (numNotes == 1 && (type == StrumType::Down || type == StrumType::DownAltBass)) {
            const int bassRank = (type == StrumType::DownAltBass) ? 1 : 0;
            const int bassString = nthLowestPitchedString(shapeFrets, bassRank);
            stringOrder.clear();
            if (bassString >= 0) stringOrder.push_back(bassString);
        } else if (numNotes > 0 && numNotes < (int)stringOrder.size()) {
            if (type == StrumType::Down || type == StrumType::DownAltBass) {
                std::vector<int> selected;
                selected.push_back(stringOrder.front());
                if (numNotes >= 2) {
                    selected.push_back(stringOrder.back());
                }
                for (int i = 1; (int)selected.size() < numNotes && i < (int)stringOrder.size() - 1; ++i) {
                    selected.push_back(stringOrder[i]);
                }
                stringOrder = selected;
            } else if (type == StrumType::Up || type == StrumType::ArpeggioDesc) {
                stringOrder.erase(stringOrder.begin(), stringOrder.end() - numNotes);
            } else {
                stringOrder.resize(numNotes);
            }
        }
        if (type == StrumType::ArpeggioAsc || type == StrumType::ArpeggioDesc) {
            if (!stringOrder.empty()) {
                int notes = (int)stringOrder.size();
                for (int i = 0; i < beatsPerBar; ++i) {
                    int s = stringOrder[i % notes];
                    int beatSample = evSample + i * samplesPerBeat;
                    if (beatSample < samplesPerMeasure)
                        stringAttackSchedule[s].push_back(beatSample);
                }
            }
        } else {
            double strumDuration = 0.04;
            int strumSamples = std::max(1, static_cast<int>(strumDuration * sampleRate_));
            for (size_t i = 0; i < stringOrder.size(); ++i) {
                int s = stringOrder[i];
                int attackSample = evSample + (int)(i * (strumSamples / std::max(1, (int)stringOrder.size()-1)));
                if (attackSample < samplesPerMeasure)
                    stringAttackSchedule[s].push_back(attackSample);
            }
        }
    }

    for (unsigned int f = 0; f < nFrames; ++f) {
        int absSample = measureSampleCounter + f;
        double sample = 0.0;
        for (int s = 0; s < kNumStrings; ++s) {
            int fret = (int)shapeFrets.size() > s ? shapeFrets[s] : -1;
            if (fret == -1) continue;

            bool pluck_event = false;
            for (int attack_sample : stringAttackSchedule[s]) {
                if (attack_sample == absSample % samplesPerMeasure) {
                    pluck_event = true;
                    voices_[s].lastAttackSample = absSample;
                    break;
                }
            }
            if (pluck_event) {
                double freq = midiToFreq(midiForStringFret(s, fret));
                noteOn(s, freq, absSample);
            }
            sample += nextSample(voices_[s]) * nextEnvelope(voices_[s], absSample);
        }
        out[f] = static_cast<float>(sample * volume_ / kNumStrings);
    }
}
