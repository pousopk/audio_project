#include "../include/Metronome.h"
#include "../widgets/BeatVisualizerWidget.h"
#include <QApplication>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

// Simple test window to show metronome and beat visualizer in sync
class TestMetronomeVisualWindow : public QWidget {
public:
    TestMetronomeVisualWindow(QWidget *parent = nullptr) : QWidget(parent) {
        QVBoxLayout *layout = new QVBoxLayout(this);
        visualizer = new BeatVisualizerWidget(this);
        visualizer->setMinimumHeight(120);
        layout->addWidget(visualizer);
        setLayout(layout);
        setWindowTitle("Metronome Visualizer Test");
        resize(500, 150);

        // Set up metronome
        metronome.setBpm(160);
        metronome.setBeatsPerBar(4);
        metronome.setTimeSignatureDenominator(4);
        metronome.setVolume(0.5);
        metronome.start();
        visualizer->setBeatsPerBar(4);

        // Polling timer for perfect sync
        pollTimer = new QTimer(this);
        connect(pollTimer, &QTimer::timeout, this, &TestMetronomeVisualWindow::onPoll);
        pollTimer->start(10); // Poll every 10ms

        // Timer to change time signatures
        changeTimer = new QTimer(this);
        connect(changeTimer, &QTimer::timeout, this, &TestMetronomeVisualWindow::onChangeSignature);
        changeTimer->start(20000); // 20 seconds
    }
private slots:
    void onPoll() {
        int beat = metronome.getCurrentBeatIndex();
        if (beat != lastBeat) {
            visualizer->setCurrentBeat(beat);
            lastBeat = beat;
        }
    }
private:
    Metronome metronome;
    BeatVisualizerWidget *visualizer;
    QTimer *pollTimer;
    QTimer *changeTimer;
    int lastBeat = -1;
    int signatureStep = 0;
private slots:
    void onChangeSignature() {
        signatureStep++;
        if (signatureStep == 1) {
            metronome.setBeatsPerBar(3);
            metronome.setTimeSignatureDenominator(4);
            visualizer->setBeatsPerBar(3);
            setWindowTitle("Metronome Visualizer Test - 3/4");
        } else if (signatureStep == 2) {
            metronome.setBeatsPerBar(5);
            metronome.setTimeSignatureDenominator(4);
            visualizer->setBeatsPerBar(5);
            setWindowTitle("Metronome Visualizer Test - 5/4");
        } else {
            changeTimer->stop();
        }
    }
};



int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    TestMetronomeVisualWindow window;
    window.show();
    return app.exec();
}
