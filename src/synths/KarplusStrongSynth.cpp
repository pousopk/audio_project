#include "synths/KarplusStrongSynth.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>

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

double wrapReadPos(double pos, int size) {
    while (pos < 0.0) pos += static_cast<double>(size);
    while (pos >= static_cast<double>(size)) pos -= static_cast<double>(size);
    return pos;
}

double readFractional(const std::vector<double>& line, double pos) {
    if (line.empty()) return 0.0;
    int size = static_cast<int>(line.size());
    pos = wrapReadPos(pos, size);
    int i0 = static_cast<int>(pos);
    int i1 = (i0 + 1) % size;
    double frac = pos - static_cast<double>(i0);
    return line[i0] * (1.0 - frac) + line[i1] * frac;
}
} // namespace


KarplusStrongSynth::KarplusStrongSynth() {
    updateBodyResonanceCoeffs();
}

void KarplusStrongSynth::setSampleRate(unsigned int rate) {
    sampleRate_ = rate;
    updateBodyResonanceCoeffs();
}

void KarplusStrongSynth::setVolume(double volume) {
    volume_ = volume;
}

void KarplusStrongSynth::setDecay(double decay) {
    decay_ = std::clamp(decay, 0.90, 0.9999);
}

void KarplusStrongSynth::setExciterBrightness(double brightness) {
    lossCutoffNorm_ = std::clamp(brightness, 0.02, 0.95);
}

void KarplusStrongSynth::setPickTransient(double amount) {
    transientAmount_ = std::clamp(amount, 0.0, 1.5);
}

void KarplusStrongSynth::setDispersion(double amount) {
    dispersionCoeff_ = std::clamp(amount, 0.0, 0.45);
}

void KarplusStrongSynth::setDrive(double drive) {
    nonlinearityDrive_ = std::clamp(drive, 0.2, 3.0);
}

void KarplusStrongSynth::setBodyResonance(double amount) {
    bodyGain_ = std::clamp(amount, 0.0, 1.5);
}

void KarplusStrongSynth::setSympatheticResonance(double amount) {
    sympatheticAmount_ = std::clamp(amount, 0.0, 0.2);
}

void KarplusStrongSynth::updateBodyResonanceCoeffs() {
    const double sr = std::max(1u, sampleRate_);
    const std::array<double, kBodyModes> freqs = {95.0, 190.0, 380.0};
    const std::array<double, kBodyModes> qs = {0.9, 1.3, 1.8};

    for (int m = 0; m < kBodyModes; ++m) {
        const double w0 = 2.0 * M_PI * freqs[m] / sr;
        const double alpha = std::sin(w0) / (2.0 * qs[m]);
        const double cosw = std::cos(w0);

        // Band-pass resonator (constant skirt gain)
        const double b0 = alpha;
        const double b1 = 0.0;
        const double b2 = -alpha;
        const double a0 = 1.0 + alpha;
        const double a1 = -2.0 * cosw;
        const double a2 = 1.0 - alpha;

        bodyFilters_[m].b0 = b0 / a0;
        bodyFilters_[m].b1 = b1 / a0;
        bodyFilters_[m].b2 = b2 / a0;
        bodyFilters_[m].a1 = a1 / a0;
        bodyFilters_[m].a2 = a2 / a0;
    }
}

void KarplusStrongSynth::pluck(int string, double freq, int absSample) {
    if (string < 0 || string >= kNumStrings) return;
    voices_[string].freq = freq;
    int delayLen = std::max(8, static_cast<int>(sampleRate_ / std::max(20.0, freq)));
    voices_[string].delayLine.assign(delayLen, 0.0);

    // Exciter: filtered noise burst + short bipolar pick transient + pick-position comb notch.
    const int pickOffset = std::max(1, static_cast<int>(0.15 * delayLen));
    const int transientLen = std::max(2, static_cast<int>(0.004 * sampleRate_));
    double noiseLP = 0.0;
    for (int i = 0; i < delayLen; ++i) {
        double noise = ((double)std::rand() / RAND_MAX) * 2.0 - 1.0;
        noiseLP = 0.35 * noise + 0.65 * noiseLP;
        double excitation = noiseLP;
        if (i == 0) {
            excitation += transientAmount_;
        } else if (i == 1) {
            excitation -= transientAmount_ * 0.7;
        }
        if (i >= pickOffset) {
            excitation -= 0.45 * voices_[string].delayLine[i - pickOffset];
        }
        voices_[string].delayLine[i] = excitation;
    }

    // Normalize initial burst to avoid over-energized loops.
    double sumSq = 0.0;
    for (double x : voices_[string].delayLine) {
        sumSq += x * x;
    }
    const double rms = std::sqrt(sumSq / std::max<size_t>(1, voices_[string].delayLine.size()));
    if (rms > 1e-6) {
        const double targetRms = 0.33;
        const double gain = targetRms / rms;
        for (double& x : voices_[string].delayLine) {
            x *= gain;
        }
    }
    voices_[string].writeIndex = 0;
    voices_[string].plucked = true;
    voices_[string].lastAttackSample = absSample;
    voices_[string].lossState = 0.0;
    voices_[string].dispPrevIn = 0.0;
    voices_[string].dispPrevOut = 0.0;
    voices_[string].bodyX1.fill(0.0);
    voices_[string].bodyX2.fill(0.0);
    voices_[string].bodyY1.fill(0.0);
    voices_[string].bodyY2.fill(0.0);
}

void KarplusStrongSynth::injectSympathetic(int drivingString, double drivingFreq, const std::vector<int>& shapeFrets) {
    if (drivingFreq <= 1.0) return;

    for (int s = 0; s < kNumStrings; ++s) {
        if (s == drivingString) continue;
        int fret = (int)shapeFrets.size() > s ? shapeFrets[s] : -1;
        if (fret < 0) continue;

        const double targetFreq = midiToFreq(midiForStringFret(s, fret));
        const double ratio = targetFreq / drivingFreq;
        const int harmonic = std::max(1, static_cast<int>(std::round(ratio)));
        const double detune = std::abs(ratio - harmonic);
        const double coupling = 0.2 * sympatheticAmount_ * std::exp(-10.0 * detune) / harmonic;
        if (coupling < 1e-4) continue;

        Voice& v = voices_[s];
        if (v.delayLine.empty()) {
            const int delayLen = std::max(8, static_cast<int>(sampleRate_ / std::max(20.0, targetFreq)));
            v.delayLine.assign(delayLen, 0.0);
            v.writeIndex = 0;
            v.freq = targetFreq;
        }
        if (!v.delayLine.empty()) {
            v.delayLine[v.writeIndex] += coupling;
        }
        v.plucked = true;
        v.freq = targetFreq;
    }
}

double KarplusStrongSynth::nextSample(Voice& v) {
    if (v.delayLine.empty()) return 0.0;

    const int size = static_cast<int>(v.delayLine.size());
    const double desiredDelay = std::clamp(static_cast<double>(sampleRate_) / std::max(20.0, v.freq), 2.0, static_cast<double>(size - 2));

    // Fractional delay read.
    const double readPos = static_cast<double>(v.writeIndex) - desiredDelay;
    const double loopIn = readFractional(v.delayLine, readPos);

    // Loss filter (one-pole low-pass in the loop).
    const double lossAlpha = std::clamp(lossCutoffNorm_, 0.01, 0.99);
    v.lossState = lossAlpha * (loopIn * decay_) + (1.0 - lossAlpha) * v.lossState;

    // Dispersion filter (first-order all-pass).
    const double a = std::clamp(dispersionCoeff_, -0.85, 0.85);
    const double dispOut = -a * v.lossState + v.dispPrevIn + a * v.dispPrevOut;
    v.dispPrevIn = v.lossState;
    v.dispPrevOut = dispOut;

    // Nonlinearity.
    const double nonlinear = std::tanh(nonlinearityDrive_ * dispOut);

    // Write back into loop.
    v.delayLine[v.writeIndex] = nonlinear;
    v.writeIndex = (v.writeIndex + 1) % size;

    // Body resonance EQ bank.
    double bodyOut = 0.0;
    for (int m = 0; m < kBodyModes; ++m) {
        const auto& f = bodyFilters_[m];
        const double y = f.b0 * nonlinear + f.b1 * v.bodyX1[m] + f.b2 * v.bodyX2[m]
                       - f.a1 * v.bodyY1[m] - f.a2 * v.bodyY2[m];
        v.bodyX2[m] = v.bodyX1[m];
        v.bodyX1[m] = nonlinear;
        v.bodyY2[m] = v.bodyY1[m];
        v.bodyY1[m] = y;
        bodyOut += bodyModeGains_[m] * y;
    }

    // Dry/body blend keeps definition while retaining body character.
    return 0.7 * nonlinear + bodyGain_ * bodyOut;
}

void KarplusStrongSynth::process(float* out,
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
                pluck(s, freq, absSample);
                injectSympathetic(s, freq, shapeFrets);
            }
            sample += nextSample(voices_[s]);
        }
        const double mixed = sample * volume_ / kNumStrings;
        out[f] = static_cast<float>(std::tanh(0.9 * mixed));
    }
}
