#include "../include/ChordAudioEngine.h"
#include "../strumming/StrummingPattern.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <vector>

// Helper: Print available strumming patterns
void printStrummingPatterns() {
    std::cout << "Available strumming patterns:\n";
    for (size_t i = 0; i < kStrummingPatterns.size(); ++i) {
        std::cout << i << ": " << kStrummingPatterns[i].name << std::endl;
    }
}

// Helper: Run a looped chord progression with selected strumming
void runChordStrumTest(int strumIdx, int bpm = 120, int numLoops = 2) {
    ChordAudioEngine engine;
    engine.setSampleRate(48000);
    engine.setBpm(bpm);
    engine.setStrummingPattern(kStrummingPatterns[strumIdx]);
    ChordProgression prog;
    // Example: C, G, A, F (1 bar each)
    prog.addChord("C", 1, strumIdx);
    prog.addChord("G", 1, strumIdx);
    prog.addChord("A", 1, strumIdx);
    prog.addChord("F", 1, strumIdx);
    engine.setProgression(prog);
    engine.start();
    for (int i = 0; i < numLoops; ++i) {
        std::cout << "Loop " << (i + 1) << "/" << numLoops << std::endl;
        // Calculate total duration of the progression
        double totalMeasures = 0;
        for (const auto& chord : prog.getChords()) {
            totalMeasures += chord.bars;
        }
        double secondsPerLoop = totalMeasures * 4.0 * 60.0 / bpm; // Assuming 4/4 time
        std::this_thread::sleep_for(std::chrono::duration<double>(secondsPerLoop));
    }
    engine.stop();
}

// Test: Play one chord (C) with each strumming pattern for 1 bar
void testAllStrumTypesOneChord(int bpm = 120) {
    ChordAudioEngine engine;
    engine.setSampleRate(48000);
    engine.setBpm(bpm);
    for (size_t i = 0; i < kStrummingPatterns.size(); ++i) {
        std::cout << "Testing pattern " << i << ": " << kStrummingPatterns[i].name << std::endl;
        engine.setStrummingPattern(kStrummingPatterns[i]);
        ChordProgression prog;
        prog.addChord("C", 1, i);
        engine.setProgression(prog);
        engine.start();
        std::this_thread::sleep_for(std::chrono::seconds(4 * 60 / bpm)); // 1 bar x 4 beats
        engine.stop();
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Short pause between patterns
    }
}

int main(int argc, char** argv) {
    printStrummingPatterns();
    bool testAll = false;
    int idx = 0;
    if (argc > 1 && std::string(argv[1]) == "--all") {
        testAll = true;
    } else {
        std::cout << "Enter strumming pattern index: ";
        std::cin >> idx;
        if (idx < 0 || idx >= (int)kStrummingPatterns.size()) {
            std::cerr << "Invalid index." << std::endl;
            return 1;
        }
    }
    if (testAll) {
        testAllStrumTypesOneChord();
    } else {
        runChordStrumTest(idx);
    }
    return 0;
}
