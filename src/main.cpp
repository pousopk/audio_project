#include "../widgets/NoteMapFretboardWidget.h"
#include "../utils/NoteMapUtils.h"
#include <array>
#include <set>
#include <vector>
#include <QString>
//
// ...existing code...
#include "../include/GuitarChordShape.h"
// Helper: get note names for a chord (root, 3rd, 5th, 7th)
std::vector<QString> getChordNotes(const QString& chordName) {
    // Only supports basic major, minor, 7, maj7, m7
    static const char* noteNames[12] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
    int rootIdx = -1;
    QString root = chordName.left(1);
    if (chordName.length() >= 2 && (chordName[1] == '#' || chordName[1] == 'b'))
        root = chordName.left(2);
    for (int i = 0; i < 12; ++i) if (root == noteNames[i]) rootIdx = i;
    if (rootIdx == -1) rootIdx = 0;
    bool isMinor = chordName.contains("m") && !chordName.contains("maj");
    bool isMaj7 = chordName.contains("maj7");
    bool isDom7 = chordName.contains("7") && !isMaj7 && !isMinor;
    bool isMin7 = chordName.contains("m7");
    int third = isMinor ? 3 : 4;
    int seventh = (isMaj7 ? 11 : (isDom7 || isMin7) ? 10 : -1);
    std::vector<QString> notes;
    notes.push_back(noteNames[rootIdx]);
    notes.push_back(noteNames[(rootIdx + third) % 12]);
    notes.push_back(noteNames[(rootIdx + 7) % 12]);
    if (seventh != -1) notes.push_back(noteNames[(rootIdx + seventh) % 12]);
    return notes;
}


// MetronomeWindow class definition (global scope)
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QComboBox>
#include <QApplication>
#include <QDebug>
#include "Metronome.h"
#include "ChordTrackWidget.h"
#include "../widgets/ChordProgressionWidget.h"

class MetronomeWindow : public QWidget {
    Q_OBJECT
public:
    MetronomeWindow(QWidget *parent = nullptr);
private slots:
    void onPlayRequested(const QVector<ChordChange>& progression);
    void onBPMChanged(int value);
    void onBeatsChanged(int value);
    void onDenomChanged(const QString &text);
    void onSubdivChanged(int value);
    void onVolumeChanged(int value);
    void onChordSelected(const QString& chordName);
private:
    ChordAudioEngine audioEngine;
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
    ChordProgressionWidget *progressionWidget;
    NoteMapFretboardWidget *noteMapFretboard;
};

// MetronomeWindow implementation (global scope)
MetronomeWindow::MetronomeWindow(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    noteMapFretboard = new NoteMapFretboardWidget(this);

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

    // Chord progression/backing track module (new UI)
    progressionWidget = new ChordProgressionWidget(this);
    layout->addWidget(progressionWidget);
    connect(progressionWidget, &ChordProgressionWidget::progressionChanged, this, [this](const QVector<ChordChange>& progression) {
        // For demo: print to console, or update other UI elements as needed
        QStringList chords;
        for (const auto& change : progression) {
            chords << QString("%1 (%2 bars)").arg(change.chordName).arg(change.bars);
        }
        qDebug() << "Current progression:" << chords.join(", ");
    });
    connect(progressionWidget, &ChordProgressionWidget::playRequested, this, &MetronomeWindow::onPlayRequested);
    connect(progressionWidget, &ChordProgressionWidget::stopRequested, this, [this]() {
        audioEngine.stop();
        metronome.reset();
    });

    // Add the note map fretboard to the main layout
    layout->addWidget(noteMapFretboard);

    // Optionally, connect progressionWidget to update fretboard/note map on chord change

    setLayout(layout);
    setWindowTitle("Metronome & Backing Track");
    resize(900, 800);
    metronome.setBpm(slider->value());
    metronome.setBeatsPerBar(beatsSlider->value());
    metronome.setSubdivisions(subdivSlider->value());
    metronome.setVolume(volumeSlider->value() / 100.0);
    // metronome.start(); // Only start metronome when not using backing track
}

void MetronomeWindow::onPlayRequested(const QVector<ChordChange>& progression) {
    // Always reinitialize audio engine and metronome for a clean start
    audioEngine.stop();
    ChordProgression prog;
    for (const auto& c : progression) {
        prog.addChord(c.chordName.toStdString(), c.bars, c.strummingPatternIndex);
    }
    audioEngine.setProgression(prog);
    audioEngine.setMetronome(&metronome);
    audioEngine.setBpm(metronome.getBpm());
    audioEngine.setSampleRate(48000);
    audioEngine.start();
}

void MetronomeWindow::onBPMChanged(int value) {
    metronome.setBpm(value);
    bpmValue->setText(QString::number(value));
}
void MetronomeWindow::onBeatsChanged(int value) {
    metronome.setBeatsPerBar(value);
    beatsValue->setText(QString::number(value));
}
void MetronomeWindow::onDenomChanged(const QString &text) {
    timeSigDenominator = text.toInt();
    metronome.setTimeSignatureDenominator(timeSigDenominator);
}
void MetronomeWindow::onSubdivChanged(int value) {
    metronome.setSubdivisions(value);
    subdivValue->setText(QString::number(value));
}
void MetronomeWindow::onVolumeChanged(int value) {
    double vol = value / 100.0;
    metronome.setVolume(vol);
    volumeValue->setText(QString::number(vol, 'f', 2));
}
void MetronomeWindow::onChordSelected(const QString& chordName) {
    // Show all chord notes on the note map fretboard
    auto chordNotes = getChordNotes(chordName);
    QString chordLabel = chordName + " note map (";
    for (size_t i = 0; i < chordNotes.size(); ++i) {
        chordLabel += chordNotes[i];
        if (i + 1 < chordNotes.size()) chordLabel += ", ";
    }
    chordLabel += ")";
    NoteMap noteMap = makeNoteMap(chordNotes, chordLabel);
    noteMapFretboard->setNoteMap(noteMap);
}

// Helper: generate all chord shapes (very basic, demo only)
std::vector<GuitarChordShape> generateChordInversions(const QString& chordName, int maxFret) {
    // For demo: slide a basic barre shape up the neck
    std::vector<GuitarChordShape> shapes;
    std::vector<QString> notes = getChordNotes(chordName);
    std::array<int,6> baseShape = {0,0,1,2,2,0};
    if (chordName.contains("m") && !chordName.contains("maj")) baseShape = {0,0,0,2,2,0}; // Em
    for (int fret = 0; fret <= maxFret-4; ++fret) {
        GuitarChordShape shape;
        shape.frets.resize(6);
        for (int s = 0; s < 6; ++s) {
            int f = baseShape[s];
            if (f == 0 && s == 5) shape.frets[s] = -1; // mute low E for demo
            else shape.frets[s] = (f == 0 ? 0 : f + fret);
        }
        // Name with notes for coloring
        shape.name = chordName + " (";
        for (size_t i = 0; i < notes.size(); ++i) {
            shape.name += notes[i];
            if (i + 1 < notes.size()) shape.name += ", ";
        }
        shape.name += ")";
        shapes.push_back(shape);
    }
    return shapes;
}

#include <QMetaObject>
#include <QThread>
#include <QTimer>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MetronomeWindow window;
    window.show();
    return app.exec();
}

#include "main.moc"
