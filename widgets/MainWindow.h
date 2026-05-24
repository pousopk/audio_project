#pragma once

#include <QWidget>
#include <vector>
#include <string>
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
class SynthSettingsWindow;

/**
 * @brief Main application window for the audio project UI.
 */
class MainWindow : public QWidget {
    Q_OBJECT
public:
    /** @brief Construct the main window. */
    MainWindow(QWidget *parent = nullptr);

private slots:
    // UI event handlers for all controls
    void onPlayRequested(const std::vector<ChordChange>& progression);
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
    void onCompressorKneeChanged(int value);
    void onCompressorSaturationChanged(int value);
    void onCompressorDetectorBlendChanged(int value);
    void onEQLowGainChanged(int value);
    void onEQMidGainChanged(int value);
    void onEQHighGainChanged(int value);
    void onEQSaturationEnabledChanged(bool enabled);
    void onEQSaturationDriveChanged(int value);
    void onEQMSAmountChanged(int value);
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
    void refreshFretboardDisplay();

    ChordAudioEngine audioEngine; ///< Audio engine instance
    Metronome metronome; ///< Metronome instance
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
    QComboBox *synthCombo;
    ChordProgressionWidget *progressionWidget;
    NoteMapFretboardWidget *noteMapFretboard;
    EffectsWindow *effectsWindow;
    SynthSettingsWindow* synthSettingsWindow = nullptr;
    QTimer* meterUpdateTimer_;
    ColorLegendsWith *legendWidget;
    bool showScaleOverlay_ = true;
    std::string currentChordName_;
    std::vector<std::string> currentScaleNotes_;
    std::string currentScaleLabel_;
};