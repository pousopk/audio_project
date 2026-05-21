#pragma once

#include <QWidget>
#include "Metronome.h"
#include "ChordAudioEngine.h"

class QSlider;
class QLabel;
class QComboBox;
class QTimer;
class ChordProgressionWidget;
class NoteMapFretboardWidget;
class ColorLegendsWith;
class FXChainWidget;
class EffectsWindow;

class MainWindow : public QWidget {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void onPlayRequested(const QVector<ChordChange>& progression);
    void onBPMChanged(int value);
    void onBeatsChanged(int value);
    void onDenomChanged(const QString &text);
    void onSubdivChanged(int value);
    void onVolumeChanged(int value);
    void onChordSelected(const QString& chordName);
    void onReverbMixChanged(int value);
    void onDelayTimeChanged(int value);
    void onDelayFeedbackChanged(int value);
    void onDelayMixChanged(int value);
    void onDistortionDriveChanged(int value);
    void onDistortionMixChanged(int value);
    void onCompressorThresholdChanged(int value);
    void onCompressorRatioChanged(int value);
    void onCompressorAttackChanged(int value);
    void onCompressorReleaseChanged(int value);
    void onEQLowGainChanged(int value);
    void onEQMidGainChanged(int value);
    void onEQHighGainChanged(int value);
    void onLimiterThresholdChanged(int value);
    void onLimiterReleaseChanged(int value);
    void onChorusRateChanged(int value);
    void onChorusDepthChanged(int value);
    void onChorusMixChanged(int value);
    void onGateThresholdChanged(int value);
    void onGateReleaseChanged(int value);
    void onFlangerRateChanged(int value);
    void onFlangerDepthChanged(int value);
    void onFlangerFeedbackChanged(int value);
    void onFlangerMixChanged(int value);
    void onPhaserRateChanged(int value);
    void onPhaserDepthChanged(int value);
    void onPhaserFeedbackChanged(int value);
    void onPhaserMixChanged(int value);
    void onTremoloRateChanged(int value);
    void onTremoloDepthChanged(int value);
    void onScaleSelected(const QString& scaleRoot, const QString& scaleName);
    void onUpdateMeters();

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
    EffectsWindow *effectsWindow;
    QTimer* meterUpdateTimer_;
    ColorLegendsWith *legendWidget;
    std::vector<QString> currentScaleNotes_;
    QString currentScaleLabel_;
};