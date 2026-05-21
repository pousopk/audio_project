#include "EffectsWindow.h"
#include "FXChainWidget.h"
#include "GainReductionMeter.h"
#include "SpectrumAnalyzerWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QSlider>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>

EffectsWindow::EffectsWindow(QWidget* parent) : QWidget(parent, Qt::Window) {
    setWindowTitle("Effects Rack");
    setMinimumWidth(400);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // --- Spectrum Analyzer ---
    spectrumAnalyzer_ = new SpectrumAnalyzerWidget(this);
    mainLayout->addWidget(spectrumAnalyzer_);


    // --- Genre Templates ---
    QGroupBox* templatesGroup = new QGroupBox("Genre Templates", this);
    QHBoxLayout* templatesLayout = new QHBoxLayout();
    QComboBox* genreTemplatesCombo = new QComboBox(this);
    genreTemplatesCombo->addItem("Default");
    genreTemplatesCombo->addItem("Acoustic");
    genreTemplatesCombo->addItem("Rock");
    genreTemplatesCombo->addItem("Ambient");
    connect(genreTemplatesCombo, &QComboBox::currentTextChanged, this, &EffectsWindow::applyGenreTemplate);
    templatesLayout->addWidget(new QLabel("Load Template:"));
    templatesLayout->addWidget(genreTemplatesCombo);
    templatesGroup->setLayout(templatesLayout);
    mainLayout->addWidget(templatesGroup);

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
    reverbMixLayout->addWidget(new QLabel("Mix:", this));
    reverbSlider_ = new QSlider(Qt::Horizontal, this);
    reverbSlider_->setRange(0, 100);
    QLabel* reverbValue = new QLabel("0.00", this);
    connect(reverbSlider_, &QSlider::valueChanged, this, &EffectsWindow::reverbMixChanged);
    connect(reverbSlider_, &QSlider::valueChanged, reverbValue, [reverbValue](int value){ reverbValue->setText(QString::number(value / 100.0, 'f', 2)); });
    reverbMixLayout->addWidget(reverbSlider_);
    reverbMixLayout->addWidget(reverbValue);
    reverbGroupLayout->addLayout(reverbMixLayout);
    reverbGroup_->setLayout(reverbGroupLayout);
    paramsLayout->addWidget(reverbGroup_);

    // Delay controls
    delayGroup_ = new QGroupBox("Delay", this);
    QVBoxLayout* delayGroupLayout = new QVBoxLayout();
    QHBoxLayout* delayTimeLayout = new QHBoxLayout();
    delayTimeLayout->addWidget(new QLabel("Time (ms):", this));
    delayTimeSlider_ = new QSlider(Qt::Horizontal, this);
    delayTimeSlider_->setRange(1, 1000);
    QLabel* delayTimeValue = new QLabel("1", this);
    connect(delayTimeSlider_, &QSlider::valueChanged, this, &EffectsWindow::delayTimeChanged);
    connect(delayTimeSlider_, &QSlider::valueChanged, delayTimeValue, [delayTimeValue](int value){ delayTimeValue->setNum(value); });
    delayTimeLayout->addWidget(delayTimeSlider_);
    delayTimeLayout->addWidget(delayTimeValue);
    delayGroupLayout->addLayout(delayTimeLayout);

    QHBoxLayout* delayFeedbackLayout = new QHBoxLayout();
    delayFeedbackLayout->addWidget(new QLabel("Feedback:", this));
    delayFeedbackSlider_ = new QSlider(Qt::Horizontal, this);
    delayFeedbackSlider_->setRange(0, 99);
    QLabel* delayFeedbackValue = new QLabel("0.00", this);
    connect(delayFeedbackSlider_, &QSlider::valueChanged, this, &EffectsWindow::delayFeedbackChanged);
    connect(delayFeedbackSlider_, &QSlider::valueChanged, delayFeedbackValue, [delayFeedbackValue](int value){ delayFeedbackValue->setText(QString::number(value / 100.0, 'f', 2)); });
    delayFeedbackLayout->addWidget(delayFeedbackSlider_);
    delayFeedbackLayout->addWidget(delayFeedbackValue);
    delayGroupLayout->addLayout(delayFeedbackLayout);

    QHBoxLayout* delayMixLayout = new QHBoxLayout();
    delayMixLayout->addWidget(new QLabel("Mix:", this));
    delayMixSlider_ = new QSlider(Qt::Horizontal, this);
    delayMixSlider_->setRange(0, 100);
    QLabel* delayMixValue = new QLabel("0.00", this);
    connect(delayMixSlider_, &QSlider::valueChanged, this, &EffectsWindow::delayMixChanged);
    connect(delayMixSlider_, &QSlider::valueChanged, delayMixValue, [delayMixValue](int value){ delayMixValue->setText(QString::number(value / 100.0, 'f', 2)); });
    delayMixLayout->addWidget(delayMixSlider_);
    delayMixLayout->addWidget(delayMixValue);
    delayGroupLayout->addLayout(delayMixLayout);
    delayGroup_->setLayout(delayGroupLayout);
    paramsLayout->addWidget(delayGroup_);

    // Distortion controls
    distortionGroup_ = new QGroupBox("Distortion", this);
    QVBoxLayout* distortionGroupLayout = new QVBoxLayout();
    QHBoxLayout* distortionDriveLayout = new QHBoxLayout();
    distortionDriveLayout->addWidget(new QLabel("Drive:", this));
    distortionDriveSlider_ = new QSlider(Qt::Horizontal, this);
    distortionDriveSlider_->setRange(0, 100);
    QLabel* distortionDriveValue = new QLabel("0.00", this);
    connect(distortionDriveSlider_, &QSlider::valueChanged, this, &EffectsWindow::distortionDriveChanged);
    connect(distortionDriveSlider_, &QSlider::valueChanged, distortionDriveValue, [distortionDriveValue](int value){ distortionDriveValue->setText(QString::number(value / 100.0, 'f', 2)); });
    distortionDriveLayout->addWidget(distortionDriveSlider_);
    distortionDriveLayout->addWidget(distortionDriveValue);
    distortionGroupLayout->addLayout(distortionDriveLayout);

    QHBoxLayout* distortionMixLayout = new QHBoxLayout();
    distortionMixLayout->addWidget(new QLabel("Mix:", this));
    distortionMixSlider_ = new QSlider(Qt::Horizontal, this);
    distortionMixSlider_->setRange(0, 100);
    QLabel* distortionMixValue = new QLabel("1.00", this);
    connect(distortionMixSlider_, &QSlider::valueChanged, this, &EffectsWindow::distortionMixChanged);
    connect(distortionMixSlider_, &QSlider::valueChanged, distortionMixValue, [distortionMixValue](int value){ distortionMixValue->setText(QString::number(value / 100.0, 'f', 2)); });
    distortionMixLayout->addWidget(distortionMixSlider_);
    distortionMixLayout->addWidget(distortionMixValue);
    distortionGroupLayout->addLayout(distortionMixLayout);

    distortionGroup_->setLayout(distortionGroupLayout);
    paramsLayout->addWidget(distortionGroup_);

    // Compressor controls
    compressorGroup_ = new QGroupBox("Compressor", this);
    QVBoxLayout* compressorGroupLayout = new QVBoxLayout();

    QHBoxLayout* thresholdLayout = new QHBoxLayout();
    thresholdLayout->addWidget(new QLabel("Threshold (dB):", this));
    compressorThresholdSlider_ = new QSlider(Qt::Horizontal, this);
    compressorThresholdSlider_->setRange(-60, 0);
    QLabel* thresholdValue = new QLabel("0", this);
    connect(compressorThresholdSlider_, &QSlider::valueChanged, this, &EffectsWindow::compressorThresholdChanged);
    connect(compressorThresholdSlider_, &QSlider::valueChanged, thresholdValue, [thresholdValue](int value){ thresholdValue->setNum(value); });
    thresholdLayout->addWidget(compressorThresholdSlider_);
    thresholdLayout->addWidget(thresholdValue);
    compressorGroupLayout->addLayout(thresholdLayout);

    QHBoxLayout* ratioLayout = new QHBoxLayout();
    ratioLayout->addWidget(new QLabel("Ratio:", this));
    compressorRatioSlider_ = new QSlider(Qt::Horizontal, this);
    compressorRatioSlider_->setRange(1, 20);
    QLabel* ratioValue = new QLabel("1:1", this);
    connect(compressorRatioSlider_, &QSlider::valueChanged, this, &EffectsWindow::compressorRatioChanged);
    connect(compressorRatioSlider_, &QSlider::valueChanged, ratioValue, [ratioValue](int value){ ratioValue->setText(QString::number(value) + ":1"); });
    ratioLayout->addWidget(compressorRatioSlider_);
    ratioLayout->addWidget(ratioValue);
    compressorGroupLayout->addLayout(ratioLayout);

    QHBoxLayout* attackLayout = new QHBoxLayout();
    attackLayout->addWidget(new QLabel("Attack (ms):", this));
    compressorAttackSlider_ = new QSlider(Qt::Horizontal, this);
    compressorAttackSlider_->setRange(1, 200);
    QLabel* attackValue = new QLabel("1", this);
    connect(compressorAttackSlider_, &QSlider::valueChanged, this, &EffectsWindow::compressorAttackChanged);
    connect(compressorAttackSlider_, &QSlider::valueChanged, attackValue, [attackValue](int value){ attackValue->setNum(value); });
    attackLayout->addWidget(compressorAttackSlider_);
    attackLayout->addWidget(attackValue);
    compressorGroupLayout->addLayout(attackLayout);

    QHBoxLayout* releaseLayout = new QHBoxLayout();
    releaseLayout->addWidget(new QLabel("Release (ms):", this));
    compressorReleaseSlider_ = new QSlider(Qt::Horizontal, this);
    compressorReleaseSlider_->setRange(10, 1000);
    QLabel* releaseValue = new QLabel("10", this);
    connect(compressorReleaseSlider_, &QSlider::valueChanged, this, &EffectsWindow::compressorReleaseChanged);
    connect(compressorReleaseSlider_, &QSlider::valueChanged, releaseValue, [releaseValue](int value){ releaseValue->setNum(value); });
    releaseLayout->addWidget(compressorReleaseSlider_);
    releaseLayout->addWidget(releaseValue);
    compressorGroupLayout->addLayout(releaseLayout);

    // Add the meter to the layout
    compressorMeter_ = new GainReductionMeter(this);
    compressorGroupLayout->addWidget(compressorMeter_, 0, Qt::AlignRight);

    compressorGroup_->setLayout(compressorGroupLayout);
    paramsLayout->addWidget(compressorGroup_);

    // EQ controls
    eqGroup_ = new QGroupBox("3-Band EQ", this);
    QVBoxLayout* eqGroupLayout = new QVBoxLayout();

    QHBoxLayout* lowGainLayout = new QHBoxLayout();
    lowGainLayout->addWidget(new QLabel("Low Gain (dB):", this));
    eqLowGainSlider_ = new QSlider(Qt::Horizontal, this);
    eqLowGainSlider_->setRange(-15, 15);
    QLabel* lowGainValue = new QLabel("0", this);
    connect(eqLowGainSlider_, &QSlider::valueChanged, this, &EffectsWindow::eqLowGainChanged);
    connect(eqLowGainSlider_, &QSlider::valueChanged, lowGainValue, [lowGainValue](int value){ lowGainValue->setNum(value); });
    lowGainLayout->addWidget(eqLowGainSlider_);
    lowGainLayout->addWidget(lowGainValue);
    eqGroupLayout->addLayout(lowGainLayout);

    QHBoxLayout* midGainLayout = new QHBoxLayout();
    midGainLayout->addWidget(new QLabel("Mid Gain (dB):", this));
    eqMidGainSlider_ = new QSlider(Qt::Horizontal, this);
    eqMidGainSlider_->setRange(-15, 15);
    QLabel* midGainValue = new QLabel("0", this);
    connect(eqMidGainSlider_, &QSlider::valueChanged, this, &EffectsWindow::eqMidGainChanged);
    connect(eqMidGainSlider_, &QSlider::valueChanged, midGainValue, [midGainValue](int value){ midGainValue->setNum(value); });
    midGainLayout->addWidget(eqMidGainSlider_);
    midGainLayout->addWidget(midGainValue);
    eqGroupLayout->addLayout(midGainLayout);

    QHBoxLayout* highGainLayout = new QHBoxLayout();
    highGainLayout->addWidget(new QLabel("High Gain (dB):", this));
    eqHighGainSlider_ = new QSlider(Qt::Horizontal, this);
    eqHighGainSlider_->setRange(-15, 15);
    QLabel* highGainValue = new QLabel("0", this);
    connect(eqHighGainSlider_, &QSlider::valueChanged, this, &EffectsWindow::eqHighGainChanged);
    connect(eqHighGainSlider_, &QSlider::valueChanged, highGainValue, [highGainValue](int value){ highGainValue->setNum(value); });
    highGainLayout->addWidget(eqHighGainSlider_);
    highGainLayout->addWidget(highGainValue);
    eqGroupLayout->addLayout(highGainLayout);

    eqGroup_->setLayout(eqGroupLayout);
    paramsLayout->addWidget(eqGroup_);

    // Limiter controls
    limiterGroup_ = new QGroupBox("Limiter", this);
    QVBoxLayout* limiterGroupLayout = new QVBoxLayout();

    QHBoxLayout* limThresholdLayout = new QHBoxLayout();
    limThresholdLayout->addWidget(new QLabel("Threshold (dB):", this));
    limiterThresholdSlider_ = new QSlider(Qt::Horizontal, this);
    limiterThresholdSlider_->setRange(-200, 0); // Range in 0.1 dB steps
    QLabel* limThresholdValue = new QLabel("0.0", this);
    connect(limiterThresholdSlider_, &QSlider::valueChanged, this, &EffectsWindow::limiterThresholdChanged);
    connect(limiterThresholdSlider_, &QSlider::valueChanged, limThresholdValue, [limThresholdValue](int value){ limThresholdValue->setText(QString::number(value / 10.0, 'f', 1)); });
    limThresholdLayout->addWidget(limiterThresholdSlider_);
    limThresholdLayout->addWidget(limThresholdValue);
    limiterGroupLayout->addLayout(limThresholdLayout);

    QHBoxLayout* limReleaseLayout = new QHBoxLayout();
    limReleaseLayout->addWidget(new QLabel("Release (ms):", this));
    limiterReleaseSlider_ = new QSlider(Qt::Horizontal, this);
    limiterReleaseSlider_->setRange(10, 500);
    QLabel* limReleaseValue = new QLabel("10", this);
    connect(limiterReleaseSlider_, &QSlider::valueChanged, this, &EffectsWindow::limiterReleaseChanged);
    connect(limiterReleaseSlider_, &QSlider::valueChanged, limReleaseValue, [limReleaseValue](int value){ limReleaseValue->setNum(value); });
    limReleaseLayout->addWidget(limiterReleaseSlider_);
    limReleaseLayout->addWidget(limReleaseValue);
    limiterGroupLayout->addLayout(limReleaseLayout);

    limiterGroup_->setLayout(limiterGroupLayout);
    paramsLayout->addWidget(limiterGroup_);

    // Chorus controls
    chorusGroup_ = new QGroupBox("Chorus", this);
    QVBoxLayout* chorusGroupLayout = new QVBoxLayout();

    QHBoxLayout* chorusRateLayout = new QHBoxLayout();
    chorusRateLayout->addWidget(new QLabel("Rate (Hz):", this));
    chorusRateSlider_ = new QSlider(Qt::Horizontal, this);
    chorusRateSlider_->setRange(1, 100); // 0.1 to 10.0 Hz
    QLabel* chorusRateValue = new QLabel("0.1", this);
    connect(chorusRateSlider_, &QSlider::valueChanged, this, &EffectsWindow::chorusRateChanged);
    connect(chorusRateSlider_, &QSlider::valueChanged, chorusRateValue, [chorusRateValue](int value){ chorusRateValue->setText(QString::number(value / 10.0, 'f', 1)); });
    chorusRateLayout->addWidget(chorusRateSlider_);
    chorusRateLayout->addWidget(chorusRateValue);
    chorusGroupLayout->addLayout(chorusRateLayout);

    QHBoxLayout* chorusDepthLayout = new QHBoxLayout();
    chorusDepthLayout->addWidget(new QLabel("Depth (ms):", this));
    chorusDepthSlider_ = new QSlider(Qt::Horizontal, this);
    chorusDepthSlider_->setRange(1, 50); // 0.1 to 5.0 ms
    QLabel* chorusDepthValue = new QLabel("0.1", this);
    connect(chorusDepthSlider_, &QSlider::valueChanged, this, &EffectsWindow::chorusDepthChanged);
    connect(chorusDepthSlider_, &QSlider::valueChanged, chorusDepthValue, [chorusDepthValue](int value){ chorusDepthValue->setText(QString::number(value / 10.0, 'f', 1)); });
    chorusDepthLayout->addWidget(chorusDepthSlider_);
    chorusDepthLayout->addWidget(chorusDepthValue);
    chorusGroupLayout->addLayout(chorusDepthLayout);

    QHBoxLayout* chorusMixLayout = new QHBoxLayout();
    chorusMixLayout->addWidget(new QLabel("Mix:", this));
    chorusMixSlider_ = new QSlider(Qt::Horizontal, this);
    chorusMixSlider_->setRange(0, 100);
    QLabel* chorusMixValue = new QLabel("0.00", this);
    connect(chorusMixSlider_, &QSlider::valueChanged, this, &EffectsWindow::chorusMixChanged);
    connect(chorusMixSlider_, &QSlider::valueChanged, chorusMixValue, [chorusMixValue](int value){ chorusMixValue->setText(QString::number(value / 100.0, 'f', 2)); });
    chorusMixLayout->addWidget(chorusMixSlider_);
    chorusMixLayout->addWidget(chorusMixValue);
    chorusGroupLayout->addLayout(chorusMixLayout);

    chorusGroup_->setLayout(chorusGroupLayout);
    paramsLayout->addWidget(chorusGroup_);

    // Gate controls
    gateGroup_ = new QGroupBox("Gate", this);
    QVBoxLayout* gateGroupLayout = new QVBoxLayout();

    QHBoxLayout* gateThresholdLayout = new QHBoxLayout();
    gateThresholdLayout->addWidget(new QLabel("Threshold (dB):", this));
    gateThresholdSlider_ = new QSlider(Qt::Horizontal, this);
    gateThresholdSlider_->setRange(-80, 0);
    QLabel* gateThresholdValue = new QLabel("-80", this);
    connect(gateThresholdSlider_, &QSlider::valueChanged, this, &EffectsWindow::gateThresholdChanged);
    connect(gateThresholdSlider_, &QSlider::valueChanged, gateThresholdValue, [gateThresholdValue](int value){ gateThresholdValue->setNum(value); });
    gateThresholdLayout->addWidget(gateThresholdSlider_);
    gateThresholdLayout->addWidget(gateThresholdValue);
    gateGroupLayout->addLayout(gateThresholdLayout);

    QHBoxLayout* gateReleaseLayout = new QHBoxLayout();
    gateReleaseLayout->addWidget(new QLabel("Release (ms):", this));
    gateReleaseSlider_ = new QSlider(Qt::Horizontal, this);
    gateReleaseSlider_->setRange(10, 500);
    QLabel* gateReleaseValue = new QLabel("10", this);
    connect(gateReleaseSlider_, &QSlider::valueChanged, this, &EffectsWindow::gateReleaseChanged);
    connect(gateReleaseSlider_, &QSlider::valueChanged, gateReleaseValue, [gateReleaseValue](int value){ gateReleaseValue->setNum(value); });
    gateReleaseLayout->addWidget(gateReleaseSlider_);
    gateReleaseLayout->addWidget(gateReleaseValue);
    gateGroupLayout->addLayout(gateReleaseLayout);

    gateGroup_->setLayout(gateGroupLayout);
    paramsLayout->addWidget(gateGroup_);

    // Flanger controls
    flangerGroup_ = new QGroupBox("Flanger", this);
    QVBoxLayout* flangerGroupLayout = new QVBoxLayout();

    QHBoxLayout* flangerRateLayout = new QHBoxLayout();
    flangerRateLayout->addWidget(new QLabel("Rate (Hz):", this));
    flangerRateSlider_ = new QSlider(Qt::Horizontal, this);
    flangerRateSlider_->setRange(1, 50); // 0.1 to 5.0 Hz
    QLabel* flangerRateValue = new QLabel("0.1", this);
    connect(flangerRateSlider_, &QSlider::valueChanged, this, &EffectsWindow::flangerRateChanged);
    connect(flangerRateSlider_, &QSlider::valueChanged, flangerRateValue, [flangerRateValue](int value){ flangerRateValue->setText(QString::number(value / 10.0, 'f', 1)); });
    flangerRateLayout->addWidget(flangerRateSlider_);
    flangerRateLayout->addWidget(flangerRateValue);
    flangerGroupLayout->addLayout(flangerRateLayout);

    QHBoxLayout* flangerDepthLayout = new QHBoxLayout();
    flangerDepthLayout->addWidget(new QLabel("Depth (ms):", this));
    flangerDepthSlider_ = new QSlider(Qt::Horizontal, this);
    flangerDepthSlider_->setRange(1, 50); // 0.1 to 5.0 ms
    QLabel* flangerDepthValue = new QLabel("0.1", this);
    connect(flangerDepthSlider_, &QSlider::valueChanged, this, &EffectsWindow::flangerDepthChanged);
    connect(flangerDepthSlider_, &QSlider::valueChanged, flangerDepthValue, [flangerDepthValue](int value){ flangerDepthValue->setText(QString::number(value / 10.0, 'f', 1)); });
    flangerDepthLayout->addWidget(flangerDepthSlider_);
    flangerDepthLayout->addWidget(flangerDepthValue);
    flangerGroupLayout->addLayout(flangerDepthLayout);

    QHBoxLayout* flangerFeedbackLayout = new QHBoxLayout();
    flangerFeedbackLayout->addWidget(new QLabel("Feedback:", this));
    flangerFeedbackSlider_ = new QSlider(Qt::Horizontal, this);
    flangerFeedbackSlider_->setRange(0, 95); // 0.0 to 0.95
    QLabel* flangerFeedbackValue = new QLabel("0.00", this);
    connect(flangerFeedbackSlider_, &QSlider::valueChanged, this, &EffectsWindow::flangerFeedbackChanged);
    connect(flangerFeedbackSlider_, &QSlider::valueChanged, flangerFeedbackValue, [flangerFeedbackValue](int value){ flangerFeedbackValue->setText(QString::number(value / 100.0, 'f', 2)); });
    flangerFeedbackLayout->addWidget(flangerFeedbackSlider_);
    flangerFeedbackLayout->addWidget(flangerFeedbackValue);
    flangerGroupLayout->addLayout(flangerFeedbackLayout);

    // Mix control is shared with Chorus for simplicity in this example, but could be separate
    // For now, we'll just add a mix slider to the Flanger group

    flangerGroup_->setLayout(flangerGroupLayout);
    paramsLayout->addWidget(flangerGroup_);

    // Phaser controls
    phaserGroup_ = new QGroupBox("Phaser", this);
    QVBoxLayout* phaserGroupLayout = new QVBoxLayout();

    QHBoxLayout* phaserRateLayout = new QHBoxLayout();
    phaserRateLayout->addWidget(new QLabel("Rate (Hz):", this));
    phaserRateSlider_ = new QSlider(Qt::Horizontal, this);
    phaserRateSlider_->setRange(1, 50); // 0.1 to 5.0 Hz
    QLabel* phaserRateValue = new QLabel("0.1", this);
    connect(phaserRateSlider_, &QSlider::valueChanged, this, &EffectsWindow::phaserRateChanged);
    connect(phaserRateSlider_, &QSlider::valueChanged, phaserRateValue, [phaserRateValue](int value){ phaserRateValue->setText(QString::number(value / 10.0, 'f', 1)); });
    phaserRateLayout->addWidget(phaserRateSlider_);
    phaserRateLayout->addWidget(phaserRateValue);
    phaserGroupLayout->addLayout(phaserRateLayout);

    QHBoxLayout* phaserDepthLayout = new QHBoxLayout();
    phaserDepthLayout->addWidget(new QLabel("Depth:", this));
    phaserDepthSlider_ = new QSlider(Qt::Horizontal, this);
    phaserDepthSlider_->setRange(1, 100);
    QLabel* phaserDepthValue = new QLabel("0.01", this);
    connect(phaserDepthSlider_, &QSlider::valueChanged, this, &EffectsWindow::phaserDepthChanged);
    connect(phaserDepthSlider_, &QSlider::valueChanged, phaserDepthValue, [phaserDepthValue](int value){ phaserDepthValue->setText(QString::number(value / 100.0, 'f', 2)); });
    phaserDepthLayout->addWidget(phaserDepthSlider_);
    phaserDepthLayout->addWidget(phaserDepthValue);
    phaserGroupLayout->addLayout(phaserDepthLayout);

    QHBoxLayout* phaserFeedbackLayout = new QHBoxLayout();
    phaserFeedbackLayout->addWidget(new QLabel("Feedback:", this));
    phaserFeedbackSlider_ = new QSlider(Qt::Horizontal, this);
    phaserFeedbackSlider_->setRange(0, 95); // 0.0 to 0.95
    QLabel* phaserFeedbackValue = new QLabel("0.00", this);
    connect(phaserFeedbackSlider_, &QSlider::valueChanged, this, &EffectsWindow::phaserFeedbackChanged);
    connect(phaserFeedbackSlider_, &QSlider::valueChanged, phaserFeedbackValue, [phaserFeedbackValue](int value){ phaserFeedbackValue->setText(QString::number(value / 100.0, 'f', 2)); });
    phaserFeedbackLayout->addWidget(phaserFeedbackSlider_);
    phaserFeedbackLayout->addWidget(phaserFeedbackValue);
    phaserGroupLayout->addLayout(phaserFeedbackLayout);

    // A mix control could be added here, similar to other effects

    phaserGroup_->setLayout(phaserGroupLayout);
    paramsLayout->addWidget(phaserGroup_);

    // Tremolo controls
    tremoloGroup_ = new QGroupBox("Tremolo", this);
    QVBoxLayout* tremoloGroupLayout = new QVBoxLayout();

    QHBoxLayout* tremoloRateLayout = new QHBoxLayout();
    tremoloRateLayout->addWidget(new QLabel("Rate (Hz):", this));
    tremoloRateSlider_ = new QSlider(Qt::Horizontal, this);
    tremoloRateSlider_->setRange(1, 200); // 0.1 to 20.0 Hz
    QLabel* tremoloRateValue = new QLabel("0.1", this);
    connect(tremoloRateSlider_, &QSlider::valueChanged, this, &EffectsWindow::tremoloRateChanged);
    connect(tremoloRateSlider_, &QSlider::valueChanged, tremoloRateValue, [tremoloRateValue](int value){ tremoloRateValue->setText(QString::number(value / 10.0, 'f', 1)); });
    tremoloRateLayout->addWidget(tremoloRateSlider_);
    tremoloRateLayout->addWidget(tremoloRateValue);
    tremoloGroupLayout->addLayout(tremoloRateLayout);

    QHBoxLayout* tremoloDepthLayout = new QHBoxLayout();
    tremoloDepthLayout->addWidget(new QLabel("Depth:", this));
    tremoloDepthSlider_ = new QSlider(Qt::Horizontal, this);
    tremoloDepthSlider_->setRange(0, 100);
    QLabel* tremoloDepthValue = new QLabel("0.00", this);
    connect(tremoloDepthSlider_, &QSlider::valueChanged, this, &EffectsWindow::tremoloDepthChanged);
    connect(tremoloDepthSlider_, &QSlider::valueChanged, tremoloDepthValue, [tremoloDepthValue](int value){ tremoloDepthValue->setText(QString::number(value / 100.0, 'f', 2)); });
    tremoloDepthLayout->addWidget(tremoloDepthSlider_);
    tremoloDepthLayout->addWidget(tremoloDepthValue);
    tremoloGroupLayout->addLayout(tremoloDepthLayout);

    tremoloGroup_->setLayout(tremoloGroupLayout);
    paramsLayout->addWidget(tremoloGroup_);

    paramsGroup->setLayout(paramsLayout);
    mainLayout->addWidget(paramsGroup);
    mainLayout->addStretch(); // Pushes everything up
    setLayout(mainLayout);
    updateEffectControls({}); // Initially hide all controls
}

void EffectsWindow::applyGenreTemplate(const QString& genre) {
    if (genre == "Acoustic") {
        reverbSlider_->setValue(20);
        delayMixSlider_->setValue(0);
        distortionDriveSlider_->setValue(0);
        compressorThresholdSlider_->setValue(-12);
        compressorRatioSlider_->setValue(3);
        eqMidGainSlider_->setValue(-3); // Scoop mids for clarity
        limiterThresholdSlider_->setValue(-10); // -1.0 dB
        chorusMixSlider_->setValue(30);
        gateThresholdSlider_->setValue(-60);
        fxChainWidget_->setOrder({"Gate", "Compressor", "EQ", "Chorus", "Reverb", "Limiter"}); // Flanger/Phaser/Tremolo not in default acoustic
    } else if (genre == "Rock") {
        reverbSlider_->setValue(35);
        delayTimeSlider_->setValue(250);
        delayFeedbackSlider_->setValue(30);
        delayMixSlider_->setValue(25);
        distortionDriveSlider_->setValue(40);
        distortionMixSlider_->setValue(100);
        compressorThresholdSlider_->setValue(-18);
        compressorRatioSlider_->setValue(4);
        eqLowGainSlider_->setValue(2);
        eqHighGainSlider_->setValue(3);
        limiterThresholdSlider_->setValue(-20); // -2.0 dB
        gateThresholdSlider_->setValue(-50);
        flangerRateSlider_->setValue(2); // 0.2 Hz
        flangerDepthSlider_->setValue(20); // 2.0 ms
        tremoloRateSlider_->setValue(60); // 6.0 Hz
        tremoloDepthSlider_->setValue(80);
        phaserRateSlider_->setValue(4); // 0.4 Hz
        phaserDepthSlider_->setValue(80);
        fxChainWidget_->setOrder({"Gate", "Compressor", "Distortion", "EQ", "Phaser", "Tremolo", "Delay", "Reverb", "Limiter"});
    } else if (genre == "Ambient") {
        reverbSlider_->setValue(80);
        delayTimeSlider_->setValue(750);
        delayFeedbackSlider_->setValue(60);
        delayMixSlider_->setValue(50);
        distortionDriveSlider_->setValue(10);
        distortionMixSlider_->setValue(50);
        compressorThresholdSlider_->setValue(-24);
        compressorRatioSlider_->setValue(2);
        eqLowGainSlider_->setValue(-6); // Cut lows to avoid mud
        eqHighGainSlider_->setValue(4);
        chorusRateSlider_->setValue(5); // 0.5 Hz
        chorusDepthSlider_->setValue(35); // 3.5 ms
        chorusMixSlider_->setValue(60);
        limiterThresholdSlider_->setValue(-10); // -1.0 dB
        fxChainWidget_->setOrder({"Gate", "Chorus", "Delay", "Reverb", "Limiter"}); // No flanger/phaser/tremolo for ambient
    } else { // Default
        reverbSlider_->setValue(30);
        distortionDriveSlider_->setValue(0);
        distortionMixSlider_->setValue(100);
        delayTimeSlider_->setValue(500);
        delayFeedbackSlider_->setValue(50);
        delayMixSlider_->setValue(30);
        compressorThresholdSlider_->setValue(-20);
        compressorRatioSlider_->setValue(4);
        compressorAttackSlider_->setValue(10);
        compressorReleaseSlider_->setValue(100);
        eqLowGainSlider_->setValue(0);
        eqMidGainSlider_->setValue(0);
        eqHighGainSlider_->setValue(0);
        chorusRateSlider_->setValue(5);
        chorusDepthSlider_->setValue(25);
        chorusMixSlider_->setValue(50);
        gateThresholdSlider_->setValue(-70);
        gateReleaseSlider_->setValue(100);
        flangerRateSlider_->setValue(3);
        flangerDepthSlider_->setValue(15);
        flangerFeedbackSlider_->setValue(70);
        phaserRateSlider_->setValue(5);
        phaserDepthSlider_->setValue(90);
        phaserFeedbackSlider_->setValue(50);
        tremoloRateSlider_->setValue(50); // 5.0 Hz
        tremoloDepthSlider_->setValue(100);
        limiterThresholdSlider_->setValue(-10); // -1.0 dB
        limiterReleaseSlider_->setValue(50);
        fxChainWidget_->setOrder({"Gate", "Phaser", "Flanger", "Chorus", "Tremolo", "Delay", "Reverb", "Limiter"});
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


void EffectsWindow::setReverbMix(float mix) { reverbSlider_->setValue(static_cast<int>(mix * 100)); }
void EffectsWindow::setDistortionMix(float mix) { distortionMixSlider_->setValue(static_cast<int>(mix * 100)); }
void EffectsWindow::setDelayTime(float time_ms) { delayTimeSlider_->setValue(static_cast<int>(time_ms)); }
void EffectsWindow::setDelayFeedback(float feedback) { delayFeedbackSlider_->setValue(static_cast<int>(feedback * 100)); }
void EffectsWindow::setDelayMix(float mix) { delayMixSlider_->setValue(static_cast<int>(mix * 100)); }
void EffectsWindow::setFXOrder(const QStringList& order) { fxChainWidget_->setOrder(order); }

void EffectsWindow::updateCompressorMeter(float db) {
    compressorMeter_->setGainReduction(db);
}

void EffectsWindow::updateSpectrum(const std::vector<float>& spectrumData) {
    spectrumAnalyzer_->setData(spectrumData);
}