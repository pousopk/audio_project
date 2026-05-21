
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

/**
 * @brief Widget for building and playing a chord track.
 */
class ChordTrackWidget : public QWidget {
    Q_OBJECT
public:
    /** @brief Construct a ChordTrackWidget. */
    ChordTrackWidget(QWidget *parent = nullptr);
    /** @brief Set the metronome instance. */
    void setMetronome(class Metronome* m);
    /** @brief Set the chord selector text. */
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
