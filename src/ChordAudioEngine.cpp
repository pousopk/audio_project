#include "../strumming/StrummingPattern.h"
#include <atomic>
#include "ChordAudioEngine.h"
#include "Metronome.h"
#include <cmath>
#include <cstring>
#include <map>
#include "../include/GuitarChordShape.h"
#include "../utils/GuitarChordShapes.h"
#include <QString>
#include <vector>
#include <array>
#include <iostream>

// Static variables for chord sync state
int g_measureSampleCounter = 0;
int g_chordMeasureCount = 0;
bool g_started = false;

void resetChordSyncState() {
    g_measureSampleCounter = 0;
    g_chordMeasureCount = 0;
    g_started = false;
}


void ChordAudioEngine::setMetronome(Metronome* m) {
    metronome = m;
}

ChordAudioEngine::ChordAudioEngine() {}
ChordAudioEngine::~ChordAudioEngine() { stop(); }

void ChordAudioEngine::setProgression(const ChordProgression& prog) {
    progression = prog;
    currentChordIndex = 0;
    samplesLeftInChord = 0;
}

void ChordAudioEngine::setBpm(double b) { bpm = b; }
void ChordAudioEngine::setSampleRate(unsigned int rate) { sampleRate = rate; }
bool ChordAudioEngine::isRunning() const { return running; }

void ChordAudioEngine::start() {
    if (running) return;
    RtAudio::StreamParameters params;
    params.deviceId = dac.getDefaultOutputDevice();
    params.nChannels = 1;
    params.firstChannel = 0;
    unsigned int bufferFrames = 256;
    running = true;
    // Reset static state in fillBuffer so playback always aligns with the next measure boundary
    resetChordSyncState();
    // --- Always wait for the next strong beat (start of a new bar) before playing the first chord ---
    if (metronome) {
        int beatsPerBar = metronome->getBeatsPerBar();
        int denominator = metronome->getTimeSignatureDenominator();
        double bpmToUse = metronome->getBpm();
        double noteLength = 4.0 / denominator;
        double secondsPerBeat = (60.0 / bpmToUse) * noteLength;
        int samplesPerBeat = static_cast<int>(secondsPerBeat * sampleRate);
        int samplesPerMeasure = samplesPerBeat * beatsPerBar;
        int beatIdx = metronome->getCurrentBeatIndex();
        // Wait until the next beat 0 (strong beat)
        if (beatIdx == 0) {
            g_measureSampleCounter = 0;
        } else {
            g_measureSampleCounter = samplesPerBeat * (beatsPerBar - beatIdx);
        }
    }
    try {
        dac.openStream(&params, nullptr, RTAUDIO_FLOAT32, sampleRate, &bufferFrames, &audioCallback, this);
        dac.startStream();
    } catch (const std::exception& e) {
        std::cerr << "RtAudio error: " << e.what() << std::endl;
        running = false;
    }
}

void ChordAudioEngine::stop() {
    running = false;
    if (dac.isStreamRunning()) dac.stopStream();
    if (dac.isStreamOpen()) dac.closeStream();
    // Reset static state so next start aligns with measure boundary
    resetChordSyncState();
}
// Helper to reset static sync state in fillBuffer


int ChordAudioEngine::audioCallback(void* outputBuffer, void*, unsigned int nBufferFrames, double, RtAudioStreamStatus status, void* userData) {
    auto* engine = static_cast<ChordAudioEngine*>(userData);
    float* out = static_cast<float*>(outputBuffer);
    if (status) std::cerr << "Stream under/overrun detected." << std::endl;
    engine->fillBuffer(out, nBufferFrames);
    return engine->running ? 0 : 1;
}

// Simple block chord synthesis: root, third, fifth, (seventh if exists)
// Supports C, C#, Db, D, D#, Eb, ..., B, Bb, etc.

// Standard tuning MIDI for each string
static const std::array<int, 6> stringMidi = {40, 45, 50, 55, 59, 64};
// Helper: get MIDI number for a string/fret
static int getMidiForStringFret(int string, int fret) {
    return stringMidi[string] + fret;
}
// Helper: MIDI to frequency
static double midiToFreq(int midi) {
    return 440.0 * std::pow(2.0, (midi - 69) / 12.0);
}

void ChordAudioEngine::fillBuffer(float* out, unsigned int nFrames) {
    // Prepare chord audio
    const auto& chords = progression.getChords();
    if (chords.empty()) {
        for (unsigned int i = 0; i < nFrames; ++i) out[i] = 0.0f;
        return;
    }

    // Synthesis parameters
    const int numHarmonics = 6; // Number of harmonics for richer timbre
    const double amp = 5.5 * chordVolume; // Overall amplitude, scaled by user volume (much louder)
    const double attack = 0.008; // faster attack
    const double release = 0.18; // longer release
    const double detuneCents = 4.0; // subtle detune per string
    static bool phasesInitialized = false;
    static std::array<double, 6> phases = {0,0,0,0,0,0};
    static std::array<double, 6> detuneRatios = {1,1,1,1,1,1};
    if (!phasesInitialized) {
        for (int s = 0; s < 6; ++s) {
            phases[s] = ((double)rand() / RAND_MAX) * 2 * M_PI;
            double cents = ((double)rand() / RAND_MAX - 0.5) * 2 * detuneCents;
            detuneRatios[s] = std::pow(2.0, cents / 1200.0);
        }
        phasesInitialized = true;
    }

    // --- Synchronize chord changes to measure boundaries ---
    // Always use metronome's BPM, beats per bar, and denominator for timing
    int beatsPerBar = 4;
    int denominator = 4;
    double bpmToUse = bpm;
    if (metronome) {
        beatsPerBar = metronome->getBeatsPerBar();
        denominator = metronome->getTimeSignatureDenominator();
        bpmToUse = metronome->getBpm();
    }
    double noteLength = 4.0 / denominator;
    double secondsPerBeat = (60.0 / bpmToUse) * noteLength;
    int samplesPerBeat = static_cast<int>(secondsPerBeat * sampleRate);
    int samplesPerMeasure = samplesPerBeat * beatsPerBar;
    int& measureSampleCounter = g_measureSampleCounter;
    int& chordMeasureCount = g_chordMeasureCount;
    bool& started = g_started;
    unsigned int frame = 0;
    static int lastChordIndex = -1;
    while (frame < nFrames) {
        // Wait until measureSampleCounter == 0 (the very first sample of a measure) to start playback
        if (!started) {
            if (measureSampleCounter == 0) {
                started = true;
            } else {
                int silenceFrames = std::min(static_cast<int>(nFrames - frame), samplesPerMeasure - measureSampleCounter);
                for (int f = 0; f < silenceFrames; ++f) {
                    out[frame + f] = 0.0f;
                }
                frame += silenceFrames;
                measureSampleCounter += silenceFrames;
                if (measureSampleCounter >= samplesPerMeasure) {
                    measureSampleCounter = 0;
                }
                continue;
            }
        }
        if (currentChordIndex >= static_cast<int>(chords.size())) {
            currentChordIndex = 0; // loop back to first chord
            lastChordIndex = -1;   // force callback for first chord of new loop
        }
        const ChordEntry& chord = chords[currentChordIndex];
        // --- Chord change event ---
        if (lastChordIndex != currentChordIndex && chordChangeCallback) {
            chordChangeCallback(chord.chordName);
            lastChordIndex = currentChordIndex;
        }
        if (chordMeasureCount <= 0) {
            chordMeasureCount = chord.measures;
        }
        int samplesLeftInMeasure = samplesPerMeasure - measureSampleCounter;
        int framesThisMeasure = std::min(static_cast<int>(nFrames - frame), samplesLeftInMeasure);
        // --- Use lowest available guitar inversion for this chord ---
        // Use the same logic as generateChordInversions in main.cpp
        std::vector<int> shapeFrets;
        auto it = kGuitarChordShapes.find(chord.chordName);
        if (it != kGuitarChordShapes.end()) {
            shapeFrets = it->second;
        } else {
            // fallback: all muted
            shapeFrets = {-1, -1, -1, -1, -1, -1};
        }
        // Strumming/arpeggio synthesis (per-chord pattern)
        static std::array<double, 6> phases = {0,0,0,0,0,0};
        static std::array<double, 6> stringEnv = {0,0,0,0,0,0}; // Envelope per string
        static std::array<int, 6> lastAttackSample = {0,0,0,0,0,0};
        // Use the strumming pattern for this chord
        int strumIdx = chord.strummingPatternIndex;
        const auto& events = (strumIdx >= 0 && strumIdx < (int)kStrummingPatterns.size()) ? kStrummingPatterns[strumIdx].events : strummingPattern.events;
        int measureStartSample = measureSampleCounter;
        int measureEndSample = measureSampleCounter + framesThisMeasure;
        // Precompute all string attack times for this measure
        std::vector<std::vector<int>> stringAttackSchedule(6); // stringAttackSchedule[s] = list of attack samples for string s
        for (const auto& ev : events) {
            int evSample = static_cast<int>(ev.position * samplesPerMeasure);
            if (evSample < 0 || evSample >= samplesPerMeasure) continue;
            StrumType type = ev.type;
            int numNotes = ev.numNotes;
            std::vector<int> stringOrder;
            if (type == StrumType::Down || type == StrumType::ArpeggioAsc) {
                for (int s = 0; s < 6; ++s) if ((int)shapeFrets.size() > s && shapeFrets[s] != -1) stringOrder.push_back(s);
            } else if (type == StrumType::Up || type == StrumType::ArpeggioDesc) {
                for (int s = 5; s >= 0; --s) if ((int)shapeFrets.size() > s && shapeFrets[s] != -1) stringOrder.push_back(s);
            }
            if (numNotes > 0 && numNotes < (int)stringOrder.size()) {
                if (type == StrumType::Down || type == StrumType::ArpeggioAsc)
                    stringOrder.resize(numNotes);
                else
                    stringOrder = std::vector<int>(stringOrder.begin(), stringOrder.begin() + numNotes);
            }
            if (type == StrumType::ArpeggioAsc || type == StrumType::ArpeggioDesc) {
                // Schedule one note per beat, cycling through strings if needed
                int notes = (int)stringOrder.size();
                int beats = beatsPerBar;
                for (int i = 0; i < beats; ++i) {
                    int s = stringOrder.empty() ? 0 : stringOrder[i % notes];
                    int beatSample = evSample + i * samplesPerBeat;
                    if (beatSample < samplesPerMeasure)
                        stringAttackSchedule[s].push_back(beatSample);
                }
            } else {
                // Strum: stagger over short duration
                double strumDuration = 0.04; // 40ms for strum
                int strumSamples = std::max(1, static_cast<int>(strumDuration * sampleRate));
                for (size_t i = 0; i < stringOrder.size(); ++i) {
                    int s = stringOrder[i];
                    int attackSample = evSample + (int)(i * (strumSamples / std::max(1, (int)stringOrder.size()-1)));
                    if (attackSample < samplesPerMeasure)
                        stringAttackSchedule[s].push_back(attackSample);
                }
            }
        }
        // For each string, keep track of the next attack index
        std::array<size_t, 6> nextAttackIdx = {0,0,0,0,0,0};
        for (int f = 0; f < framesThisMeasure; ++f) {
            int absSample = measureSampleCounter + f;
            // Trigger string attacks at their scheduled times
            for (int s = 0; s < 6; ++s) {
                while (nextAttackIdx[s] < stringAttackSchedule[s].size() && stringAttackSchedule[s][nextAttackIdx[s]] == absSample % samplesPerMeasure) {
                    lastAttackSample[s] = absSample;
                    nextAttackIdx[s]++;
                }
            }
            // Synthesize sample
            double sample = 0.0;
            int numActiveStrings = 0;
            for (int s = 0; s < 6; ++s) {
                int fret = (int)shapeFrets.size() > s ? shapeFrets[s] : -1;
                if (fret == -1) continue;
                // Envelope: sustain for full bar or half bar depending on strumming pattern
                int attackPos = absSample - lastAttackSample[s];
                double env = 0.0;
                int sustainEnd = samplesPerMeasure - static_cast<int>(release * sampleRate);
                int halfBarSamples = samplesPerMeasure / 2;
                bool isHalfBarPattern = false;
                bool isFullBarSustain = false;
                // Only half-bar and full-bar patterns sustain
                if (strummingPattern.name.find("Half") != std::string::npos) {
                    isHalfBarPattern = true;
                } else if (
                    strummingPattern.name.find("Full Bar") != std::string::npos
                ) {
                    isFullBarSustain = true;
                }
                if (isHalfBarPattern) {
                    // For half-bar patterns, sustain for half a bar after attack, then release
                    int barPos = absSample % samplesPerMeasure;
                    int halfBarStart = (barPos < halfBarSamples) ? 0 : halfBarSamples;
                    int sustainEndHalf = halfBarStart + halfBarSamples - static_cast<int>(release * sampleRate);
                    if (attackPos >= 0 && attackPos < static_cast<int>(attack * sampleRate)) {
                        env = static_cast<double>(attackPos) / (attack * sampleRate);
                    } else if (attackPos >= static_cast<int>(attack * sampleRate) && barPos < sustainEndHalf) {
                        env = 1.0;
                    } else if (barPos >= sustainEndHalf && barPos < halfBarStart + halfBarSamples) {
                        int releasePos = barPos - sustainEndHalf;
                        env = 1.0 - (static_cast<double>(releasePos) / (release * sampleRate));
                    }
                } else if (isFullBarSustain) {
                    // For full-bar patterns, sustain for full bar after attack, then release at measure end
                    if (attackPos >= 0 && attackPos < static_cast<int>(attack * sampleRate)) {
                        env = static_cast<double>(attackPos) / (attack * sampleRate);
                    } else if (attackPos >= static_cast<int>(attack * sampleRate) && (absSample % samplesPerMeasure) < sustainEnd) {
                        env = 1.0;
                    } else if ((absSample % samplesPerMeasure) >= sustainEnd && (absSample % samplesPerMeasure) < samplesPerMeasure) {
                        // Release phase at end of measure
                        int releasePos = (absSample % samplesPerMeasure) - sustainEnd;
                        env = 1.0 - (static_cast<double>(releasePos) / (release * sampleRate));
                    }
                } else {
                    // For short patterns (Down on 1, Down-Up, etc): short envelope only
                    if (attackPos >= 0 && attackPos < static_cast<int>(attack * sampleRate)) {
                        env = static_cast<double>(attackPos) / (attack * sampleRate);
                    } else if (attackPos >= static_cast<int>(attack * sampleRate) && attackPos < static_cast<int>((attack + release) * sampleRate)) {
                        env = 1.0 - (static_cast<double>(attackPos - attack * sampleRate) / (release * sampleRate));
                    }
                }
                if (env < 0.0) env = 0.0;
                if (env > 1.0) env = 1.0;
                double freq = midiToFreq(getMidiForStringFret(s, fret)) * detuneRatios[s];
                double stringAmp = 1.0 - 0.08 * s;
                numActiveStrings++;
                for (int h = 1; h <= numHarmonics; ++h) {
                    double harmAmp = 1.0 / h;
                    sample += stringAmp * harmAmp * std::sin(h * phases[s]) * env;
                }
            }
            if (numActiveStrings > 0)
                sample /= (numActiveStrings * numHarmonics);
            out[frame + f] = static_cast<float>(amp * sample);
            // Advance phases for each string
            for (int s = 0; s < 6; ++s) {
                int fret = shapeFrets.size() > s ? shapeFrets[s] : -1;
                if (fret == -1) continue;
                double freq = midiToFreq(getMidiForStringFret(s, fret)) * detuneRatios[s];
                phases[s] += 2 * M_PI * freq / sampleRate;
                if (phases[s] > 2 * M_PI) phases[s] -= 2 * M_PI;
            }
        }
        frame += framesThisMeasure;
        measureSampleCounter += framesThisMeasure;
        if (measureSampleCounter >= samplesPerMeasure) {
            measureSampleCounter = 0;
            chordMeasureCount--;
            if (chordMeasureCount <= 0) {
                currentChordIndex++;
            }
        }
    }
    // Mix in metronome if present
    if (metronome) {
        std::vector<float> metroBuf(nFrames, 0.0f);
        metronome->fillBuffer(metroBuf.data(), nFrames);
        for (unsigned int i = 0; i < nFrames; ++i) {
            out[i] += metroBuf[i];
        }
    }
}
