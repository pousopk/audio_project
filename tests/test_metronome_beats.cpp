#include <QCoreApplication>
#include <QDebug>
#include "Metronome.h"
#include <thread>
#include "ChordAudioEngine.h"
#include <chrono>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    Metronome metronome;
    metronome.setBpm(120);
    metronome.setBeatsPerBar(4);
    metronome.setTimeSignatureDenominator(4);

    // Use the audio engine to drive the metronome
    ChordAudioEngine engine;
    engine.setMetronome(&metronome);
    engine.start();


    int beatsPerBar = 4;
    int totalBeats = 10 * beatsPerBar;
    int lastBeat = -1;
    int bar = 1;
    int beatsPrinted = 0;
    while (beatsPrinted < totalBeats) {
        int beat = metronome.getCurrentBeatIndex();
        if (beat != lastBeat) {
            if (beat == 0 && beatsPrinted > 0) {
                bar++;
            }
            qDebug() << "Bar:" << bar << "Beat:" << beat;
            lastBeat = beat;
            beatsPrinted++;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    engine.stop();
    return 0;
}
