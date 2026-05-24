#include "EffectsWindow.h"
#include "FXChainWidget.h"
#include "GainReductionMeter.h"
#include "SpectrumAnalyzerWidget.h"
#include <functional>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QSlider>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>

EffectsWindow::EffectsWindow(QWidget* parent) : QWidget(parent, Qt::Window) {
    setWindowTitle("Effects Rack");
    setMinimumWidth(520);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    auto addSliderRow = [this](QVBoxLayout* layout,
                               const QString& label,
                               int min,
                               int max,
                               int initial,
                               const std::function<QString(int)>& format,
                               const std::function<void(int)>& onChanged) -> QSlider* {
        QHBoxLayout* row = new QHBoxLayout();
        row->addWidget(new QLabel(label, this));
        QSlider* slider = new QSlider(Qt::Horizontal, this);
        slider->setRange(min, max);
        QLabel* valueLabel = new QLabel(format(initial), this);
        connect(slider, &QSlider::valueChanged, this, [valueLabel, format, onChanged](int v) {
            valueLabel->setText(format(v));
            onChanged(v);
        });
        row->addWidget(slider, 1);
        row->addWidget(valueLabel);
        layout->addLayout(row);
        slider->setValue(initial);
        return slider;
    };

    // --- Spectrum Analyzer ---
    spectrumAnalyzer_ = new SpectrumAnalyzerWidget(this);
    mainLayout->addWidget(spectrumAnalyzer_);

    // --- Genre Templates ---
    QGroupBox* templatesGroup = new QGroupBox("Genre Templates", this);
    QHBoxLayout* templatesLayout = new QHBoxLayout();
    QComboBox* genreTemplatesCombo = new QComboBox(this);
    genreTemplatesCombo->addItem("Default");
    genreTemplatesCombo->addItem("Acoustic");
    genreTemplatesCombo->addItem("Bluegrass");
    genreTemplatesCombo->addItem("Rock");
    genreTemplatesCombo->addItem("Math Rock");
    genreTemplatesCombo->addItem("Ambient");
    connect(genreTemplatesCombo, &QComboBox::currentTextChanged, this, &EffectsWindow::applyGenreTemplate);
    templatesLayout->addWidget(new QLabel("Load Template:"));
    templatesLayout->addWidget(genreTemplatesCombo);
    templatesGroup->setLayout(templatesLayout);
    mainLayout->addWidget(templatesGroup);

    // --- Toggle FX Chain ---
    toggleFXButton_ = new QPushButton("Disable FX Chain", this);
    connect(toggleFXButton_, &QPushButton::clicked, this, &EffectsWindow::toggleFXChain);
    mainLayout->addWidget(toggleFXButton_);

    // --- Effect Management ---
    QGroupBox* manageGroup = new QGroupBox("Chain Management", this);
    QVBoxLayout* manageLayout = new QVBoxLayout();

    // Add/Remove controls
    QHBoxLayout* addRemoveLayout = new QHBoxLayout();
    addEffectCombo_ = new QComboBox(this);
    addEffectCombo_->addItem("Delay");
    addEffectCombo_->addItem("Reverb");
    addEffectCombo_->addItem("Distortion");
    addEffectCombo_->addItem("Compressor");
    addEffectCombo_->addItem("EQ");
    addEffectCombo_->addItem("Limiter");
    addEffectCombo_->addItem("Chorus");
    addEffectCombo_->addItem("Gate");
    addEffectCombo_->addItem("Flanger");
    addEffectCombo_->addItem("Phaser");
    addEffectCombo_->addItem("Tremolo");
    QPushButton* addButton = new QPushButton("Add", this);
    QPushButton* removeButton = new QPushButton("Remove Selected", this);
    connect(addButton, &QPushButton::clicked, this, &EffectsWindow::addEffect);
    connect(removeButton, &QPushButton::clicked, this, &EffectsWindow::removeEffect);
    addRemoveLayout->addWidget(new QLabel("Add Effect:"));
    addRemoveLayout->addWidget(addEffectCombo_);
    addRemoveLayout->addWidget(addButton);
    addRemoveLayout->addStretch();
    addRemoveLayout->addWidget(removeButton);
    manageLayout->addLayout(addRemoveLayout);

    // FX Chain ordering widget
    fxChainWidget_ = new FXChainWidget(this);
    manageLayout->addWidget(fxChainWidget_);
    connect(fxChainWidget_, &FXChainWidget::orderChanged, this, &EffectsWindow::orderChanged);
    connect(fxChainWidget_, &FXChainWidget::orderChanged, this, &EffectsWindow::updateEffectControls);

    manageGroup->setLayout(manageLayout);
    mainLayout->addWidget(manageGroup);

    // --- Effect Parameters ---
    QGroupBox* paramsGroup = new QGroupBox("Effect Parameters", this);
    QVBoxLayout* paramsLayout = new QVBoxLayout();

    // Reverb Mix controls
    reverbGroup_ = new QGroupBox("Reverb", this);
    QVBoxLayout* reverbGroupLayout = new QVBoxLayout();
    QHBoxLayout* reverbMixLayout = new QHBoxLayout();
    reverbMixLayout->addWidget(new QLabel("Mix:"));
    reverbSlider_ = new QSlider(Qt::Horizontal, this);
    reverbSlider_->setRange(0, 100);
    QLabel* reverbValue = new QLabel("0.00", this);
    connect(reverbSlider_, &QSlider::valueChanged, this, &EffectsWindow::reverbMixChanged);
    connect(reverbSlider_, &QSlider::valueChanged, reverbValue, [reverbValue](int value) {
        reverbValue->setText(QString::number(value / 100.0, 'f', 2));
    });
    reverbMixLayout->addWidget(reverbSlider_);
    reverbMixLayout->addWidget(reverbValue);
    reverbGroupLayout->addLayout(reverbMixLayout);
    reverbGroup_->setLayout(reverbGroupLayout);
    paramsLayout->addWidget(reverbGroup_);

    // Delay controls
    delayGroup_ = new QGroupBox("Delay", this);
    {
        QVBoxLayout* l = new QVBoxLayout();
        delayTimeSlider_ = addSliderRow(l, "Time (ms)", 1, 1200, 500,
                                        [](int v) { return QString::number(v); },
                                        [this](int v) { emit delayTimeChanged(v); });
        delayFeedbackSlider_ = addSliderRow(l, "Feedback", 0, 95, 50,
                                            [](int v) { return QString::number(v / 100.0, 'f', 2); },
                                            [this](int v) { emit delayFeedbackChanged(v); });
        delayMixSlider_ = addSliderRow(l, "Mix", 0, 100, 30,
                                       [](int v) { return QString::number(v / 100.0, 'f', 2); },
                                       [this](int v) { emit delayMixChanged(v); });
        delayGroup_->setLayout(l);
    }
    paramsLayout->addWidget(delayGroup_);

    // Distortion controls
    distortionGroup_ = new QGroupBox("Distortion", this);
    {
        QVBoxLayout* l = new QVBoxLayout();
        distortionDriveSlider_ = addSliderRow(l, "Drive", 0, 100, 0,
                                              [](int v) { return QString::number(v / 100.0, 'f', 2); },
                                              [this](int v) { emit distortionDriveChanged(v); });
        distortionMixSlider_ = addSliderRow(l, "Mix", 0, 100, 0,
                                            [](int v) { return QString::number(v / 100.0, 'f', 2); },
                                            [this](int v) { emit distortionMixChanged(v); });
        distortionGroup_->setLayout(l);
    }
    paramsLayout->addWidget(distortionGroup_);

    // Compressor controls
    compressorGroup_ = new QGroupBox("Compressor", this);
    {
        QVBoxLayout* l = new QVBoxLayout();
        compressorThresholdSlider_ = addSliderRow(l, "Threshold (dB)", -60, 0, -18,
                                                  [](int v) { return QString::number(v); },
                                                  [this](int v) { emit compressorThresholdChanged(v); });
        compressorRatioSlider_ = addSliderRow(l, "Ratio", 1, 20, 4,
                                              [](int v) { return QString::number(v); },
                                              [this](int v) { emit compressorRatioChanged(v); });
        compressorAttackSlider_ = addSliderRow(l, "Attack (ms)", 1, 200, 10,
                                               [](int v) { return QString::number(v); },
                                               [this](int v) { emit compressorAttackChanged(v); });
        compressorReleaseSlider_ = addSliderRow(l, "Release (ms)", 10, 1000, 150,
                                                [](int v) { return QString::number(v); },
                                                [this](int v) { emit compressorReleaseChanged(v); });
        compressorKneeSlider_ = addSliderRow(l, "Knee (dB)", 0, 24, 6,
                             [](int v) { return QString::number(v); },
                             [this](int v) { emit compressorKneeChanged(v); });
        compressorSaturationSlider_ = addSliderRow(l, "Saturation", 5, 40, 12,
                               [](int v) { return QString::number(v / 10.0, 'f', 1); },
                               [this](int v) { emit compressorSaturationChanged(v); });
        compressorDetectorBlendSlider_ = addSliderRow(l, "Peak/RMS", 0, 100, 60,
                                  [](int v) { return QString::number(v / 100.0, 'f', 2); },
                                  [this](int v) { emit compressorDetectorBlendChanged(v); });

        QHBoxLayout* meterRow = new QHBoxLayout();
        meterRow->addWidget(new QLabel("Gain Reduction", this));
        compressorMeter_ = new GainReductionMeter(this);
        meterRow->addWidget(compressorMeter_, 1);
        l->addLayout(meterRow);

        compressorGroup_->setLayout(l);
    }
    paramsLayout->addWidget(compressorGroup_);

    // EQ controls
    eqGroup_ = new QGroupBox("EQ", this);
    {
        QVBoxLayout* l = new QVBoxLayout();
        eqLowGainSlider_ = addSliderRow(l, "Low (dB)", -24, 24, 0,
                                        [](int v) { return QString::number(v); },
                                        [this](int v) { emit eqLowGainChanged(v); });
        eqMidGainSlider_ = addSliderRow(l, "Mid (dB)", -24, 24, 0,
                                        [](int v) { return QString::number(v); },
                                        [this](int v) { emit eqMidGainChanged(v); });
        eqHighGainSlider_ = addSliderRow(l, "High (dB)", -24, 24, 0,
                                         [](int v) { return QString::number(v); },
                                         [this](int v) { emit eqHighGainChanged(v); });

        QHBoxLayout* satToggleRow = new QHBoxLayout();
        satToggleRow->addWidget(new QLabel("Saturation", this));
        eqSaturationToggleButton_ = new QPushButton("ON", this);
        eqSaturationToggleButton_->setCheckable(true);
        eqSaturationToggleButton_->setChecked(true);
        connect(eqSaturationToggleButton_, &QPushButton::toggled, this, [this](bool enabled) {
            if (eqSaturationToggleButton_) {
                eqSaturationToggleButton_->setText(enabled ? "ON" : "OFF");
            }
            emit eqSaturationEnabledChanged(enabled);
        });
        satToggleRow->addWidget(eqSaturationToggleButton_);
        satToggleRow->addStretch();
        l->addLayout(satToggleRow);

        eqSaturationDriveSlider_ = addSliderRow(l, "Sat Drive", 5, 40, 12,
                                                [](int v) { return QString::number(v / 10.0, 'f', 1); },
                                                [this](int v) { emit eqSaturationDriveChanged(v); });
        eqMSAmountSlider_ = addSliderRow(l, "M/S Amount", 0, 100, 35,
                                         [](int v) { return QString::number(v / 100.0, 'f', 2); },
                                         [this](int v) { emit eqMSAmountChanged(v); });
        eqGroup_->setLayout(l);
    }
    paramsLayout->addWidget(eqGroup_);

    // Limiter controls
    limiterGroup_ = new QGroupBox("Limiter", this);
    {
        QVBoxLayout* l = new QVBoxLayout();
        limiterThresholdSlider_ = addSliderRow(l, "Threshold (dB)", -120, 0, -10,
                                               [](int v) { return QString::number(v / 10.0, 'f', 1); },
                                               [this](int v) { emit limiterThresholdChanged(v); });
        limiterReleaseSlider_ = addSliderRow(l, "Release (ms)", 10, 500, 120,
                                             [](int v) { return QString::number(v); },
                                             [this](int v) { emit limiterReleaseChanged(v); });
        limiterGroup_->setLayout(l);
    }
    paramsLayout->addWidget(limiterGroup_);

    // Chorus controls
    chorusGroup_ = new QGroupBox("Chorus", this);
    {
        QVBoxLayout* l = new QVBoxLayout();
        chorusRateSlider_ = addSliderRow(l, "Rate (Hz)", 1, 100, 4,
                                         [](int v) { return QString::number(v / 10.0, 'f', 1); },
                                         [this](int v) { emit chorusRateChanged(v); });
        chorusDepthSlider_ = addSliderRow(l, "Depth (ms)", 0, 100, 20,
                                          [](int v) { return QString::number(v / 10.0, 'f', 1); },
                                          [this](int v) { emit chorusDepthChanged(v); });
        chorusMixSlider_ = addSliderRow(l, "Mix", 0, 100, 25,
                                        [](int v) { return QString::number(v / 100.0, 'f', 2); },
                                        [this](int v) { emit chorusMixChanged(v); });
        chorusGroup_->setLayout(l);
    }

    gateGroup_ = new QGroupBox("Gate", this);
    {
        QVBoxLayout* l = new QVBoxLayout();
        gateThresholdSlider_ = addSliderRow(l, "Threshold (dB)", -80, 0, -55,
                                            [](int v) { return QString::number(v); },
                                            [this](int v) { emit gateThresholdChanged(v); });
        gateReleaseSlider_ = addSliderRow(l, "Release (ms)", 10, 1000, 200,
                                          [](int v) { return QString::number(v); },
                                          [this](int v) { emit gateReleaseChanged(v); });
        gateGroup_->setLayout(l);
    }

    flangerGroup_ = new QGroupBox("Flanger", this);
    {
        QVBoxLayout* l = new QVBoxLayout();
        flangerRateSlider_ = addSliderRow(l, "Rate (Hz)", 1, 100, 3,
                                          [](int v) { return QString::number(v / 10.0, 'f', 1); },
                                          [this](int v) { emit flangerRateChanged(v); });
        flangerDepthSlider_ = addSliderRow(l, "Depth (ms)", 0, 100, 20,
                                           [](int v) { return QString::number(v / 10.0, 'f', 1); },
                                           [this](int v) { emit flangerDepthChanged(v); });
        flangerFeedbackSlider_ = addSliderRow(l, "Feedback", 0, 95, 30,
                                              [](int v) { return QString::number(v / 100.0, 'f', 2); },
                                              [this](int v) { emit flangerFeedbackChanged(v); });
        flangerMixSlider_ = addSliderRow(l, "Mix", 0, 100, 20,
                                         [](int v) { return QString::number(v / 100.0, 'f', 2); },
                                         [this](int v) { emit flangerMixChanged(v); });
        flangerGroup_->setLayout(l);
    }

    phaserGroup_ = new QGroupBox("Phaser", this);
    {
        QVBoxLayout* l = new QVBoxLayout();
        phaserRateSlider_ = addSliderRow(l, "Rate (Hz)", 1, 100, 2,
                                         [](int v) { return QString::number(v / 10.0, 'f', 1); },
                                         [this](int v) { emit phaserRateChanged(v); });
        phaserDepthSlider_ = addSliderRow(l, "Depth", 0, 100, 35,
                                          [](int v) { return QString::number(v / 100.0, 'f', 2); },
                                          [this](int v) { emit phaserDepthChanged(v); });
        phaserFeedbackSlider_ = addSliderRow(l, "Feedback", 0, 95, 30,
                                             [](int v) { return QString::number(v / 100.0, 'f', 2); },
                                             [this](int v) { emit phaserFeedbackChanged(v); });
        phaserMixSlider_ = addSliderRow(l, "Mix", 0, 100, 18,
                                        [](int v) { return QString::number(v / 100.0, 'f', 2); },
                                        [this](int v) { emit phaserMixChanged(v); });
        phaserGroup_->setLayout(l);
    }

    tremoloGroup_ = new QGroupBox("Tremolo", this);
    {
        QVBoxLayout* l = new QVBoxLayout();
        tremoloRateSlider_ = addSliderRow(l, "Rate (Hz)", 1, 150, 50,
                                          [](int v) { return QString::number(v / 10.0, 'f', 1); },
                                          [this](int v) { emit tremoloRateChanged(v); });
        tremoloDepthSlider_ = addSliderRow(l, "Depth", 0, 100, 50,
                                           [](int v) { return QString::number(v / 100.0, 'f', 2); },
                                           [this](int v) { emit tremoloDepthChanged(v); });
        tremoloGroup_->setLayout(l);
    }

    paramsLayout->addWidget(chorusGroup_);
    paramsLayout->addWidget(gateGroup_);
    paramsLayout->addWidget(flangerGroup_);
    paramsLayout->addWidget(phaserGroup_);
    paramsLayout->addWidget(tremoloGroup_);

    paramsGroup->setLayout(paramsLayout);
    mainLayout->addWidget(paramsGroup);
    mainLayout->addStretch(); // Pushes everything up
    setLayout(mainLayout);

    updateEffectControls(fxChainWidget_->currentOrder());
}

void EffectsWindow::toggleFXChain() {
    static bool fxEnabled = true;
    fxEnabled = !fxEnabled;
    emit fxChainEnabledChanged(fxEnabled);

    if (fxEnabled) {
        toggleFXButton_->setText("Disable FX Chain");
        updateEffectControls(fxChainWidget_->currentOrder()); // Re-enable all effects
    } else {
        toggleFXButton_->setText("Enable FX Chain");
        updateEffectControls({}); // Disable all effects
    }
}

void EffectsWindow::applyGenreTemplate(const QString& genre) {
    auto setIf = [](QSlider* s, int v) {
        if (s) s->setValue(v);
    };

    if (genre == "Acoustic") {
        setIf(reverbSlider_, 20);
        setIf(delayMixSlider_, 0);
        setIf(distortionDriveSlider_, 0);
        setIf(compressorThresholdSlider_, -12);
        setIf(compressorRatioSlider_, 3);
        setIf(compressorKneeSlider_, 8);
        setIf(compressorSaturationSlider_, 10);
        setIf(compressorDetectorBlendSlider_, 45);
        setIf(eqMidGainSlider_, -3); // Scoop mids for clarity
        setIf(eqSaturationDriveSlider_, 10);
        setIf(eqMSAmountSlider_, 20);
        setIf(limiterThresholdSlider_, -10); // -1.0 dB
        setIf(chorusMixSlider_, 30);
        setIf(gateThresholdSlider_, -60);
        fxChainWidget_->setOrder({"Gate", "Compressor", "EQ", "Chorus", "Reverb", "Limiter"}); // Flanger/Phaser/Tremolo not in default acoustic
    } else if (genre == "Bluegrass") {
        setIf(reverbSlider_, 18);
        setIf(delayTimeSlider_, 120);
        setIf(delayFeedbackSlider_, 18);
        setIf(delayMixSlider_, 10);
        setIf(distortionDriveSlider_, 0);
        setIf(distortionMixSlider_, 0);
        setIf(compressorThresholdSlider_, -16);
        setIf(compressorRatioSlider_, 3);
        setIf(compressorKneeSlider_, 9);
        setIf(compressorSaturationSlider_, 10);
        setIf(compressorDetectorBlendSlider_, 40);
        setIf(eqLowGainSlider_, 1);
        setIf(eqMidGainSlider_, -1);
        setIf(eqHighGainSlider_, 2);
        setIf(eqSaturationDriveSlider_, 9);
        setIf(eqMSAmountSlider_, 18);
        setIf(chorusMixSlider_, 10);
        setIf(gateThresholdSlider_, -55);
        setIf(limiterThresholdSlider_, -12);
        fxChainWidget_->setOrder({"Gate", "Compressor", "EQ", "Reverb", "Limiter"});
    } else if (genre == "Rock") {
        setIf(reverbSlider_, 35);
        setIf(delayTimeSlider_, 250);
        setIf(delayFeedbackSlider_, 30);
        setIf(delayMixSlider_, 25);
        setIf(distortionDriveSlider_, 40);
        setIf(distortionMixSlider_, 100);
        setIf(compressorThresholdSlider_, -18);
        setIf(compressorRatioSlider_, 4);
        setIf(compressorKneeSlider_, 5);
        setIf(compressorSaturationSlider_, 16);
        setIf(compressorDetectorBlendSlider_, 75);
        setIf(eqLowGainSlider_, 2);
        setIf(eqHighGainSlider_, 3);
        setIf(eqSaturationDriveSlider_, 18);
        setIf(eqMSAmountSlider_, 45);
        setIf(limiterThresholdSlider_, -20); // -2.0 dB
        setIf(gateThresholdSlider_, -50);
        setIf(flangerRateSlider_, 2); // 0.2 Hz
        setIf(flangerDepthSlider_, 20); // 2.0 ms
        setIf(tremoloRateSlider_, 60); // 6.0 Hz
        setIf(tremoloDepthSlider_, 80);
        setIf(phaserRateSlider_, 4); // 0.4 Hz
        setIf(phaserDepthSlider_, 80);
        fxChainWidget_->setOrder({"Gate", "Compressor", "Distortion", "EQ", "Phaser", "Tremolo", "Delay", "Reverb", "Limiter"});
    } else if (genre == "Math Rock") {
        setIf(reverbSlider_, 28);
        setIf(delayTimeSlider_, 320);
        setIf(delayFeedbackSlider_, 32);
        setIf(delayMixSlider_, 22);
        setIf(distortionDriveSlider_, 10);
        setIf(distortionMixSlider_, 20);
        setIf(compressorThresholdSlider_, -20);
        setIf(compressorRatioSlider_, 4);
        setIf(compressorKneeSlider_, 7);
        setIf(compressorSaturationSlider_, 14);
        setIf(compressorDetectorBlendSlider_, 65);
        setIf(eqLowGainSlider_, -3);
        setIf(eqMidGainSlider_, 2);
        setIf(eqHighGainSlider_, 3);
        setIf(eqSaturationDriveSlider_, 16);
        setIf(eqMSAmountSlider_, 55);
        setIf(chorusRateSlider_, 4);
        setIf(chorusDepthSlider_, 20);
        setIf(chorusMixSlider_, 26);
        setIf(gateThresholdSlider_, -50);
        setIf(phaserRateSlider_, 2);
        setIf(phaserDepthSlider_, 35);
        setIf(phaserMixSlider_, 18);
        setIf(limiterThresholdSlider_, -14);
        fxChainWidget_->setOrder({"Gate", "Compressor", "EQ", "Chorus", "Delay", "Reverb", "Limiter"});
    } else if (genre == "Ambient") {
        setIf(reverbSlider_, 80);
        setIf(delayTimeSlider_, 750);
        setIf(delayFeedbackSlider_, 60);
        setIf(delayMixSlider_, 50);
        setIf(distortionDriveSlider_, 10);
        setIf(distortionMixSlider_, 50);
        setIf(compressorThresholdSlider_, -24);
        setIf(compressorRatioSlider_, 2);
        setIf(compressorKneeSlider_, 10);
        setIf(compressorSaturationSlider_, 8);
        setIf(compressorDetectorBlendSlider_, 30);
        setIf(eqLowGainSlider_, -6); // Cut lows to avoid mud
        setIf(eqHighGainSlider_, 4);
        setIf(eqSaturationDriveSlider_, 8);
        setIf(eqMSAmountSlider_, 65);
        setIf(chorusRateSlider_, 5); // 0.5 Hz
        setIf(chorusDepthSlider_, 35); // 3.5 ms
        setIf(chorusMixSlider_, 60);
        setIf(limiterThresholdSlider_, -10); // -1.0 dB
        fxChainWidget_->setOrder({"Gate", "Chorus", "Delay", "Reverb", "Limiter"}); // No flanger/phaser/tremolo for ambient
    } else { // Default
        fxChainWidget_->setOrder({}); // Explicitly clear the FX chain
        updateEffectControls({}); // Ensure UI reflects the cleared state
    }
}

void EffectsWindow::addEffect() {
    fxChainWidget_->addItem(addEffectCombo_->currentText());
}

void EffectsWindow::removeEffect() {
    int currentIndex = fxChainWidget_->currentRow();
    if (currentIndex != -1) {
        fxChainWidget_->removeItem(currentIndex);
    }
}

void EffectsWindow::updateEffectControls(const QStringList& activeEffects) {
    // Constructor-time signals can arrive before all groups are allocated.
    if (!reverbGroup_ || !delayGroup_ || !distortionGroup_ || !compressorGroup_ || !eqGroup_ ||
        !limiterGroup_ || !chorusGroup_ || !gateGroup_ || !flangerGroup_ || !phaserGroup_ || !tremoloGroup_) {
        return;
    }

    reverbGroup_->setVisible(activeEffects.contains("Reverb"));
    delayGroup_->setVisible(activeEffects.contains("Delay"));
    distortionGroup_->setVisible(activeEffects.contains("Distortion"));
    compressorGroup_->setVisible(activeEffects.contains("Compressor"));
    eqGroup_->setVisible(activeEffects.contains("EQ"));
    limiterGroup_->setVisible(activeEffects.contains("Limiter"));
    chorusGroup_->setVisible(activeEffects.contains("Chorus"));
    gateGroup_->setVisible(activeEffects.contains("Gate"));
    flangerGroup_->setVisible(activeEffects.contains("Flanger"));
    phaserGroup_->setVisible(activeEffects.contains("Phaser"));
    tremoloGroup_->setVisible(activeEffects.contains("Tremolo"));
}


void EffectsWindow::setReverbMix(float mix) {
    if (reverbSlider_) reverbSlider_->setValue(static_cast<int>(mix * 100));
}

void EffectsWindow::setDistortionMix(float mix) {
    if (distortionMixSlider_) distortionMixSlider_->setValue(static_cast<int>(mix * 100));
}

void EffectsWindow::setDelayTime(float time_ms) {
    if (delayTimeSlider_) delayTimeSlider_->setValue(static_cast<int>(time_ms));
}

void EffectsWindow::setDelayFeedback(float feedback) {
    if (delayFeedbackSlider_) delayFeedbackSlider_->setValue(static_cast<int>(feedback * 100));
}

void EffectsWindow::setDelayMix(float mix) {
    if (delayMixSlider_) delayMixSlider_->setValue(static_cast<int>(mix * 100));
}

void EffectsWindow::setFXOrder(const QStringList& order) {
    if (fxChainWidget_) fxChainWidget_->setOrder(order);
}

void EffectsWindow::updateCompressorMeter(float db) {
    if (compressorMeter_) {
        compressorMeter_->setGainReduction(db);
    }
}

void EffectsWindow::updateSpectrum(const std::vector<float>& spectrumData) {
    if (spectrumAnalyzer_) {
        spectrumAnalyzer_->setData(spectrumData);
    }
}