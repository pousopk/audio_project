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
    void eqLowGainChanged(int value);
    void eqMidGainChanged(int value);
    void eqHighGainChanged(int value);
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

private:
    QGroupBox* reverbGroup_;
    QGroupBox* delayGroup_;
    QGroupBox* distortionGroup_;
    QGroupBox* compressorGroup_;
    QGroupBox* eqGroup_;
    QGroupBox* limiterGroup_;
    QGroupBox* chorusGroup_;
    QGroupBox* gateGroup_;
    QGroupBox* flangerGroup_;
    QGroupBox* phaserGroup_;
    QGroupBox* tremoloGroup_;
    QSlider* reverbSlider_; // Now inside reverbGroup_
    QSlider* delayTimeSlider_; // Now inside delayGroup_
    QSlider* delayFeedbackSlider_; // Now inside delayGroup_
    QSlider* delayMixSlider_; // Now inside delayGroup_
    QSlider* distortionDriveSlider_; // Now inside distortionGroup_
    QSlider* distortionMixSlider_;
    QSlider* compressorThresholdSlider_;
    QSlider* compressorRatioSlider_;
    QSlider* compressorAttackSlider_;
    QSlider* compressorReleaseSlider_;
    QSlider* eqLowGainSlider_;
    QSlider* eqMidGainSlider_;
    QSlider* eqHighGainSlider_;
    QSlider* limiterThresholdSlider_;
    QSlider* limiterReleaseSlider_;
    QSlider* chorusRateSlider_;
    QSlider* chorusDepthSlider_;
    QSlider* chorusMixSlider_;
    QSlider* gateThresholdSlider_;
    QSlider* gateReleaseSlider_;
    QSlider* flangerRateSlider_;
    QSlider* flangerDepthSlider_;
    QSlider* flangerFeedbackSlider_;
    QSlider* flangerMixSlider_;
    QSlider* phaserRateSlider_;
    QSlider* phaserDepthSlider_;
    QSlider* phaserFeedbackSlider_;
    QSlider* phaserMixSlider_;
    QSlider* tremoloRateSlider_;
    QSlider* tremoloDepthSlider_;
    GainReductionMeter* compressorMeter_;
    SpectrumAnalyzerWidget* spectrumAnalyzer_;
    FXChainWidget* fxChainWidget_;
    QComboBox* addEffectCombo_;
};