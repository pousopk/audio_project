#include "../include/Metronome.h"
#include "../include/ChordAudioEngine.h"
#include "../widgets/BeatVisualizerWidget.h"
#include <QApplication>
#include <QTimer>
#include <QVBoxLayout>
#include <QMetaObject>
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

        // Set up the beat callback for perfect UI sync
        metronome.setBeatCallback([this](int beat) {
            // Safely invoke the UI update on the main GUI thread
            QMetaObject::invokeMethod(this, "onBeat", Qt::QueuedConnection, Q_ARG(int, beat));
        });

        // Use the audio engine to drive the metronome
        engine.setMetronome(&metronome);
        engine.start();

        visualizer->setBeatsPerBar(4);

        // Timer to change time signatures
        changeTimer = new QTimer(this);
        connect(changeTimer, &QTimer::timeout, this, &TestMetronomeVisualWindow::onChangeSignature);
        changeTimer->start(20000); // 20 seconds
    }
private slots:
    void onBeat(int beat) {
        visualizer->setCurrentBeat(beat);
    }
private:
    Metronome metronome;
    ChordAudioEngine engine;
    BeatVisualizerWidget *visualizer;
    QTimer *changeTimer;
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
