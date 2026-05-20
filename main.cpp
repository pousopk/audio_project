

#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QSlider>
#include <QLabel>
#include <QHBoxLayout>
#include <QComboBox>

#include "Metronome.h"
#include "ChordTrackWidget.h"

#include "widgets/BeatVisualizerWidget.h"



class BeatVisualizerWindow : public QWidget {
    Q_OBJECT
public:
    BeatVisualizerWindow(QWidget *parent = nullptr) : QWidget(parent) {
        QVBoxLayout *layout = new QVBoxLayout(this);
        beatVisualizer = new BeatVisualizerWidget(this);
        beatVisualizer->setMinimumHeight(120);
        layout->addWidget(beatVisualizer);
        setLayout(layout);
        setWindowTitle("Beat Visualizer");
        resize(500, 150);
    }
    void setBeatsPerBar(int beats) { beatVisualizer->setBeatsPerBar(beats); }
    void setCurrentBeat(int beat) { beatVisualizer->setCurrentBeat(beat); }
private:
    BeatVisualizerWidget *beatVisualizer;
};

class MetronomeWindow : public QWidget {
    Q_OBJECT
public:
    MetronomeWindow(QWidget *parent = nullptr) : QWidget(parent) {
        QVBoxLayout *layout = new QVBoxLayout(this);

        // Create separate beat visualizer window
        beatVisWindow = new BeatVisualizerWindow();
        beatVisWindow->setBeatsPerBar(metronome.getBeatsPerBar());
        beatVisWindow->show();

        // BPM controls
        QHBoxLayout *bpmLayout = new QHBoxLayout();
        QLabel *bpmLabel = new QLabel("BPM:", this);
        bpmValue = new QLabel(QString::number(metronome.getBpm()), this);
        slider = new QSlider(Qt::Horizontal, this);
        slider->setRange(40, 240);
        slider->setValue(metronome.getBpm());
        connect(slider, &QSlider::valueChanged, this, &MetronomeWindow::onBPMChanged);
        bpmLayout->addWidget(bpmLabel);
        bpmLayout->addWidget(slider);
        bpmLayout->addWidget(bpmValue);
        layout->addLayout(bpmLayout);

        // Time signature controls (numerator and denominator)
        QHBoxLayout *beatsLayout = new QHBoxLayout();
        QLabel *beatsLabel = new QLabel("Beats per bar:", this);
        beatsValue = new QLabel(QString::number(metronome.getBeatsPerBar()), this);
        beatsSlider = new QSlider(Qt::Horizontal, this);
        beatsSlider->setRange(1, 12);
        beatsSlider->setValue(metronome.getBeatsPerBar());
        connect(beatsSlider, &QSlider::valueChanged, this, &MetronomeWindow::onBeatsChanged);
        beatsLayout->addWidget(beatsLabel);
        beatsLayout->addWidget(beatsSlider);
        beatsLayout->addWidget(beatsValue);

        QLabel *denomLabel = new QLabel("/", this);
        denomCombo = new QComboBox(this);
        denomCombo->addItem("2");
        denomCombo->addItem("4");
        denomCombo->addItem("8");
        denomCombo->addItem("16");
        denomCombo->setCurrentText("4");
        connect(denomCombo, &QComboBox::currentTextChanged, this, &MetronomeWindow::onDenomChanged);
        beatsLayout->addWidget(denomLabel);
        beatsLayout->addWidget(denomCombo);

        layout->addLayout(beatsLayout);

        // Subdivision controls
        QHBoxLayout *subdivLayout = new QHBoxLayout();
        QLabel *subdivLabel = new QLabel("Subdivisions per beat:", this);
        subdivValue = new QLabel(QString::number(metronome.getSubdivisions()), this);
        subdivSlider = new QSlider(Qt::Horizontal, this);
        subdivSlider->setRange(1, 8);
        subdivSlider->setValue(metronome.getSubdivisions());
        connect(subdivSlider, &QSlider::valueChanged, this, &MetronomeWindow::onSubdivChanged);
        subdivLayout->addWidget(subdivLabel);
        subdivLayout->addWidget(subdivSlider);
        subdivLayout->addWidget(subdivValue);
        layout->addLayout(subdivLayout);

        // Volume controls
        QHBoxLayout *volumeLayout = new QHBoxLayout();
        QLabel *volumeLabel = new QLabel("Volume:", this);
        volumeValue = new QLabel(QString::number(metronome.getVolume(), 'f', 2), this);
        volumeSlider = new QSlider(Qt::Horizontal, this);
        volumeSlider->setRange(0, 100);
        volumeSlider->setValue(static_cast<int>(metronome.getVolume() * 100));
        connect(volumeSlider, &QSlider::valueChanged, this, &MetronomeWindow::onVolumeChanged);
        volumeLayout->addWidget(volumeLabel);
        volumeLayout->addWidget(volumeSlider);
        volumeLayout->addWidget(volumeValue);
        layout->addLayout(volumeLayout);


        // Chord selection controls
        QHBoxLayout *chordSelectLayout = new QHBoxLayout();
        QLabel *rootLabel = new QLabel("Root:", this);
        rootCombo = new QComboBox(this);
        // All roots with sharps and flats
        QStringList roots = {"C", "C#", "Db", "D", "D#", "Eb", "E", "F", "F#", "Gb", "G", "G#", "Ab", "A", "A#", "Bb", "B"};
        rootCombo->addItems(roots);
        QLabel *typeLabel = new QLabel("Type:", this);
        typeCombo = new QComboBox(this);
        // Common chord types
        QStringList types = {"", "m", "7", "maj7", "m7", "sus2", "sus4", "dim", "aug", "add9", "6"};
        typeCombo->addItems(types);
        chordSelectLayout->addWidget(rootLabel);
        chordSelectLayout->addWidget(rootCombo);
        chordSelectLayout->addWidget(typeLabel);
        chordSelectLayout->addWidget(typeCombo);
        layout->addLayout(chordSelectLayout);



        // Chord progression/backing track module
        QHBoxLayout *chordLayout = new QHBoxLayout();
        chordWidget = new ChordTrackWidget(this);
        chordWidget->setMetronome(&metronome);
        chordLayout->addWidget(chordWidget);
        layout->addLayout(chordLayout);

        // Connect root/type combo changes to update chordSelector
        connect(rootCombo, &QComboBox::currentTextChanged, this, &MetronomeWindow::onChordComboChanged);
        connect(typeCombo, &QComboBox::currentTextChanged, this, &MetronomeWindow::onChordComboChanged);
private slots:
    void onChordComboChanged() {
        QString chordName = rootCombo->currentText() + typeCombo->currentText();
        chordWidget->setChordSelectorText(chordName);
    }

        setLayout(layout);

        // Timer for beat visualization
        beatTimer = new QTimer(this);
        connect(beatTimer, &QTimer::timeout, this, &MetronomeWindow::onBeatTimer);
        updateBeatInterval();
        beatTimer->start(beatIntervalMs);
    }
        private slots:
            void onBeatTimer() {
                // Advance beat index
                currentBeat = (currentBeat + 1) % metronome.getBeatsPerBar();
                beatVisWindow->setCurrentBeat(currentBeat);
            }
            void onBPMChanged(int value) {
                metronome.setBpm(value);
                bpmValue->setText(QString::number(value));
                updateBeatInterval();
            }
            void onBeatsChanged(int value) {
                metronome.setBeatsPerBar(value);
                beatsValue->setText(QString::number(value));
                beatVisWindow->setBeatsPerBar(value);
                updateBeatInterval();
            }
            void updateBeatInterval() {
                // Calculate ms per beat
                double noteLength = 4.0 / metronome.getTimeSignatureDenominator();
                double msPerBeat = (60.0 / metronome.getBpm()) * noteLength * 1000.0;
                beatIntervalMs = static_cast<int>(msPerBeat);
                beatTimer->setInterval(beatIntervalMs);
            }
        setWindowTitle("Metronome & Backing Track");
        resize(600, 400);
        metronome.setBpm(slider->value());
        metronome.setBeatsPerBar(beatsSlider->value());
        metronome.setSubdivisions(subdivSlider->value());
        metronome.setVolume(volumeSlider->value() / 100.0);
        // metronome.start(); // Only start metronome when not using backing track
    }
private slots:
    void onBPMChanged(int value) {
        metronome.setBpm(value);
        bpmValue->setText(QString::number(value));
    }
    void onBeatsChanged(int value) {
        metronome.setBeatsPerBar(value);
        beatsValue->setText(QString::number(value));
    }
    void onDenomChanged(const QString &text) {
        timeSigDenominator = text.toInt();
        metronome.setTimeSignatureDenominator(timeSigDenominator);
    }
    void onSubdivChanged(int value) {
        metronome.setSubdivisions(value);
        subdivValue->setText(QString::number(value));
    }
    void onVolumeChanged(int value) {
        double vol = value / 100.0;
        metronome.setVolume(vol);
        volumeValue->setText(QString::number(vol, 'f', 2));
    }
private:
    Metronome metronome;
    QSlider *slider;
    QLabel *bpmValue;
    QSlider *beatsSlider;
    QLabel *beatsValue;
    QComboBox *denomCombo;
    int timeSigDenominator = 4;
    QSlider *subdivSlider;
    QLabel *subdivValue;
    QSlider *volumeSlider;
    QLabel *volumeValue;
    QComboBox *rootCombo;
    QComboBox *typeCombo;
    ChordTrackWidget *chordWidget;
    QTimer *beatTimer;
    int beatIntervalMs = 500;
    int currentBeat = 0;

public:
    BeatVisualizerWindow *beatVisWindow = nullptr;
};

#include <QMetaObject>
#include <QThread>
#include <QTimer>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MetronomeWindow window;
    window.show();
    // Create and show beat visualizer window as a top-level window
    window.beatVisWindow = new BeatVisualizerWindow();
    window.beatVisWindow->setBeatsPerBar(window.metronome.getBeatsPerBar());
    window.beatVisWindow->show();
    return app.exec();
}

#include "main.moc"
