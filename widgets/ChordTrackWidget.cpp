#include "../strumming/StrummingPattern.h"
#include <QSlider>
#include <QLabel>

#include "ChordTrackWidget.h"
#include "Metronome.h"
#include <QTimer>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStringList>

void ChordTrackWidget::setChordSelectorText(const QString& chordName) {
    // Try to split chordName into root and type
    static const QStringList roots = {"C", "C#", "Db", "D", "D#", "Eb", "E", "F", "F#", "Gb", "G", "G#", "Ab", "A", "A#", "Bb", "B"};
    QString foundRoot;
    QString foundType;
    for (const QString& root : roots) {
        if (chordName.startsWith(root)) {
            foundRoot = root;
            foundType = chordName.mid(root.length());
            break;
        }
    }
    if (!foundRoot.isEmpty()) {
        int rootIdx = rootCombo->findText(foundRoot);
        if (rootIdx >= 0) rootCombo->setCurrentIndex(rootIdx);
        int typeIdx = typeCombo->findText(foundType);
        if (typeIdx >= 0) typeCombo->setCurrentIndex(typeIdx);
    }
}

void ChordTrackWidget::setMetronome(Metronome* m) {
    audioEngine.setMetronome(m);
}
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStringList>



ChordTrackWidget::ChordTrackWidget(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    // Volume controls for chords
    QHBoxLayout *volumeLayout = new QHBoxLayout();
    QLabel *volumeLabel = new QLabel("Chord Volume:", this);
    volumeValue = new QLabel("1.00", this);
    volumeSlider = new QSlider(Qt::Horizontal, this);
    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(100);
    connect(volumeSlider, &QSlider::valueChanged, this, [this](int value) {
        double vol = value / 100.0;
        audioEngine.setVolume(vol);
        volumeValue->setText(QString::number(vol, 'f', 2));
    });
    volumeLayout->addWidget(volumeLabel);
    volumeLayout->addWidget(volumeSlider);
    volumeLayout->addWidget(volumeValue);
    mainLayout->addLayout(volumeLayout);
    QHBoxLayout *inputLayout = new QHBoxLayout();

    // Root and type combo boxes
    rootCombo = new QComboBox(this);
    typeCombo = new QComboBox(this);
    QStringList roots = {"C", "C#", "Db", "D", "D#", "Eb", "E", "F", "F#", "Gb", "G", "G#", "Ab", "A", "A#", "Bb", "B"};
    QStringList types = {"", "m", "7", "maj7", "m7", "sus2", "sus4", "dim", "aug", "add9", "6"};
    rootCombo->addItems(roots);
    typeCombo->addItems(types);
    inputLayout->addWidget(rootCombo);
    inputLayout->addWidget(typeCombo);

    measuresSpin = new QSpinBox(this);
    measuresSpin->setRange(1, 32);
    measuresSpin->setValue(1);
    inputLayout->addWidget(measuresSpin);

    addButton = new QPushButton("Add", this);
    inputLayout->addWidget(addButton);
    connect(addButton, &QPushButton::clicked, this, &ChordTrackWidget::onAddChord);

    removeButton = new QPushButton("Remove Selected", this);
    inputLayout->addWidget(removeButton);
    connect(removeButton, &QPushButton::clicked, this, &ChordTrackWidget::onRemoveChord);

    playButton = new QPushButton("Play", this);
    inputLayout->addWidget(playButton);
    connect(playButton, &QPushButton::clicked, this, &ChordTrackWidget::onPlay);

    stopButton = new QPushButton("Stop", this);
    inputLayout->addWidget(stopButton);
    connect(stopButton, &QPushButton::clicked, this, &ChordTrackWidget::onStop);


    // Strumming pattern selection
    QHBoxLayout *strumLayout = new QHBoxLayout();
    QLabel *strumLabel = new QLabel("Strumming Pattern:", this);
    strummingCombo = new QComboBox(this);
    for (const auto& pattern : kStrummingPatterns) {
        strummingCombo->addItem(QString::fromStdString(pattern.name));
    }
    strumLayout->addWidget(strumLabel);
    strumLayout->addWidget(strummingCombo);
    mainLayout->addLayout(strumLayout);
    mainLayout->addLayout(inputLayout);

    chordListWidget = new QListWidget(this);
    mainLayout->addWidget(chordListWidget);

    setLayout(mainLayout);
}

void ChordTrackWidget::onAddChord() {
    QString root = rootCombo->currentText();
    QString type = typeCombo->currentText();
    QString chord = root + type;
    int measures = measuresSpin->value();
    int strumIdx = strummingCombo->currentIndex();
    progression.addChord(chord.toStdString(), measures, strumIdx);
    QString strumName = strumIdx >= 0 && strumIdx < static_cast<int>(kStrummingPatterns.size()) ? QString::fromStdString(kStrummingPatterns[strumIdx].name) : "";
    chordListWidget->addItem(root + type + " (" + QString::number(measures) + " bars, " + strumName + ")");
    emit chordSelected(chord); // Emit signal for new chord
}

void ChordTrackWidget::onRemoveChord() {
    int row = chordListWidget->currentRow();
    if (row >= 0) {
        progression.removeChord(row);
        delete chordListWidget->takeItem(row);
    }
}

void ChordTrackWidget::onPlay() {
    audioEngine.setProgression(progression);
    audioEngine.setBpm(120.0); // TODO: get BPM from UI
    // Set strumming pattern from combo box
    int strumIdx = strummingCombo->currentIndex();
    if (strumIdx >= 0 && strumIdx < static_cast<int>(kStrummingPatterns.size())) {
        audioEngine.setStrummingPattern(kStrummingPatterns[strumIdx]);
    }
    // Set up callback to emit chordNowPlaying in sync with audio
    audioEngine.setChordChangeCallback([this](const std::string& chordName) {
        emit chordNowPlaying(QString::fromStdString(chordName));
    });
    audioEngine.start();
}

void ChordTrackWidget::onStop() {
    audioEngine.setChordChangeCallback(nullptr);
    audioEngine.stop();
}
