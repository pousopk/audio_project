#pragma once

#include <QWidget>
#include <QStringList>

class QSlider;
class QLabel;
class FXChainWidget;
class QGroupBox;
class QComboBox;
class GainReductionMeter;
class SpectrumAnalyzerWidget;
class QPushButton; // Add QPushButton for the toggle button

/**
 * @brief Window for controlling and visualizing audio effects parameters.
 */
class EffectsWindow : public QWidget {
    Q_OBJECT
public:
    /** @brief Construct an EffectsWindow. */
    explicit EffectsWindow(QWidget* parent = nullptr);

    // Methods to set initial values from MainWindow
    void setReverbMix(float mix);
    void setDistortionMix(float mix);
    void setDelayTime(float time_ms);
    void setDelayFeedback(float feedback);
    void setDelayMix(float mix);
    void setFXOrder(const QStringList& order);

    // Metering update
    void updateCompressorMeter(float db);
    void updateSpectrum(const std::vector<float>& spectrumData);

signals:
    void fxChainEnabledChanged(bool enabled);
    void reverbMixChanged(int value);
    void delayTimeChanged(int value);
    void delayFeedbackChanged(int value);
    void delayMixChanged(int value);
    void distortionDriveChanged(int value);
    void distortionMixChanged(int value);
    void compressorThresholdChanged(int value);
    void compressorRatioChanged(int value);
    void compressorAttackChanged(int value);
    void compressorReleaseChanged(int value);
    void compressorKneeChanged(int value);
    void compressorSaturationChanged(int value);
    void compressorDetectorBlendChanged(int value);
    void eqLowGainChanged(int value);
    void eqMidGainChanged(int value);
    void eqHighGainChanged(int value);
    void eqSaturationEnabledChanged(bool enabled);
    void eqSaturationDriveChanged(int value);
    void eqMSAmountChanged(int value);
    void limiterThresholdChanged(int value);
    void limiterReleaseChanged(int value);
    void chorusRateChanged(int value);
    void chorusDepthChanged(int value);
    void chorusMixChanged(int value);
    void gateThresholdChanged(int value);
    void gateReleaseChanged(int value);
    void flangerRateChanged(int value);
    void flangerDepthChanged(int value);
    void flangerFeedbackChanged(int value);
    void flangerMixChanged(int value);
    void phaserRateChanged(int value);
    void phaserDepthChanged(int value);
    void phaserFeedbackChanged(int value);
    void phaserMixChanged(int value);
    void tremoloRateChanged(int value);
    void tremoloDepthChanged(int value);
    void orderChanged(const QStringList& newOrder);

private slots:
    void applyGenreTemplate(const QString& genre);
    void addEffect();
    void removeEffect();
    void updateEffectControls(const QStringList& activeEffects);
    void toggleFXChain(); // Add slot for toggling the FX chain

private:
    QGroupBox* reverbGroup_ = nullptr;
    QGroupBox* delayGroup_ = nullptr;
    QGroupBox* distortionGroup_ = nullptr;
    QGroupBox* compressorGroup_ = nullptr;
    QGroupBox* eqGroup_ = nullptr;
    QGroupBox* limiterGroup_ = nullptr;
    QPushButton* toggleFXButton_ = nullptr; // Add member for the toggle button
    QSlider* reverbSlider_ = nullptr; // Now inside reverbGroup_
    QSlider* delayTimeSlider_ = nullptr; // Now inside delayGroup_
    QSlider* delayFeedbackSlider_ = nullptr; // Now inside delayGroup_
    QSlider* delayMixSlider_ = nullptr; // Now inside delayGroup_
    QSlider* distortionDriveSlider_ = nullptr; // Now inside distortionGroup_
    QSlider* distortionMixSlider_ = nullptr;
    QSlider* compressorThresholdSlider_ = nullptr;
    QSlider* compressorRatioSlider_ = nullptr;
    QSlider* compressorAttackSlider_ = nullptr;
    QSlider* compressorReleaseSlider_ = nullptr;
    QSlider* compressorKneeSlider_ = nullptr;
    QSlider* compressorSaturationSlider_ = nullptr;
    QSlider* compressorDetectorBlendSlider_ = nullptr;
    QSlider* eqLowGainSlider_ = nullptr;
    QSlider* eqMidGainSlider_ = nullptr;
    QSlider* eqHighGainSlider_ = nullptr;
    QPushButton* eqSaturationToggleButton_ = nullptr;
    QSlider* eqSaturationDriveSlider_ = nullptr;
    QSlider* eqMSAmountSlider_ = nullptr;
    QSlider* limiterThresholdSlider_ = nullptr;
    QSlider* limiterReleaseSlider_ = nullptr;
    QSlider* chorusRateSlider_ = nullptr;
    QSlider* chorusDepthSlider_ = nullptr;
    QSlider* chorusMixSlider_ = nullptr;
    QSlider* gateThresholdSlider_ = nullptr;
    QSlider* gateReleaseSlider_ = nullptr;
    QSlider* flangerRateSlider_ = nullptr;
    QSlider* flangerDepthSlider_ = nullptr;
    QSlider* flangerFeedbackSlider_ = nullptr;
    QSlider* flangerMixSlider_ = nullptr;
    QSlider* phaserRateSlider_ = nullptr;
    QSlider* phaserDepthSlider_ = nullptr;
    QSlider* phaserFeedbackSlider_ = nullptr;
    QSlider* phaserMixSlider_ = nullptr;
    QSlider* tremoloRateSlider_ = nullptr;
    QSlider* tremoloDepthSlider_ = nullptr;
    GainReductionMeter* compressorMeter_ = nullptr;
    SpectrumAnalyzerWidget* spectrumAnalyzer_ = nullptr;
    FXChainWidget* fxChainWidget_ = nullptr;
    QComboBox* addEffectCombo_ = nullptr;
    QGroupBox* chorusGroup_ = nullptr;
    QGroupBox* gateGroup_ = nullptr;
    QGroupBox* flangerGroup_ = nullptr;
    QGroupBox* phaserGroup_ = nullptr;
    QGroupBox* tremoloGroup_ = nullptr;
};