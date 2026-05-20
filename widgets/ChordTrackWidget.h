
#pragma once
#include <QWidget>
#include <QListWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include "ChordProgression.h"
#include "ChordAudioEngine.h"
#include <QTimer>

class ChordTrackWidget : public QWidget {
    Q_OBJECT
public:
    ChordTrackWidget(QWidget *parent = nullptr);
    void setMetronome(class Metronome* m);
    void setChordSelectorText(const QString& chordName);

signals:
    void chordSelected(const QString& chordName);
    void chordNowPlaying(const QString& chordName);

private slots:
    void onAddChord();
    void onRemoveChord();
    void onPlay();
    void onStop();

private:
    QComboBox *rootCombo;
    QComboBox *typeCombo;
    QSpinBox *measuresSpin;
    QPushButton *addButton;
    QPushButton *removeButton;
    QPushButton *playButton;
    QPushButton *stopButton;
    QListWidget *chordListWidget;
    QSlider *volumeSlider;
    QLabel *volumeValue;
    QComboBox *strummingCombo;
    ChordProgression progression;
    ChordAudioEngine audioEngine;
    QTimer *playbackTimer = nullptr;
    int playbackChordIndex = 0;
};
