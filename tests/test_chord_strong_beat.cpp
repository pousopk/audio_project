#include "../include/Metronome.h"
#include "../include/ChordAudioEngine.h"
#include "../include/ChordProgression.h"
#include <thread>
#include <chrono>
#include <iostream>

// This test checks that the first chord always starts on the strong beat (beat 0)
// by printing the metronome beat and a marker when the chord engine outputs sound.

int main() {
    Metronome metronome;
    metronome.setBpm(90);
    metronome.setBeatsPerBar(4);
    metronome.setTimeSignatureDenominator(4);
    metronome.setVolume(0.5); // Silence metronome click for clarity
    metronome.start();

    ChordProgression progression;
    progression.addChord("C", 1, 0); // 2 bars of C
    progression.addChord("G", 1, 0); // 2 bars of G

    ChordAudioEngine engine;
    engine.setMetronome(&metronome);
    engine.setProgression(progression);
    engine.setVolume(1.0);
    engine.setSampleRate(48000);

    // Set up a callback to print when a chord starts
    engine.setChordChangeCallback([](const std::string& chordName) {
        std::cout << "Chord: " << chordName << " (should be on beat 0)" << std::endl;
    });

    engine.start();

    // Poll metronome and print beat index for 10 bars
    int lastBeat = -1;
    int bars = 0;
    while (bars < 10) {
        int beat = metronome.getCurrentBeatIndex();
        if (beat != lastBeat) {
            std::cout << "Beat: " << beat << std::endl;
            if (beat == 0) ++bars;
            lastBeat = beat;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    engine.stop();
    metronome.stop();
    std::cout << "Test complete. Check that each 'Chord:' print aligns with 'Beat: 0'." << std::endl;
    return 0;
}
