#include "MainWindow.h"
#include "NoteMapFretboardWidget.h"
#include "ChordProgressionWidget.h"
#include "SynthSettingsWindow.h"
#include "../utils/NoteMapUtils.h"
#include "EffectsWindow.h"
#include "ColorLegendsWith.h"
#include "../utils/ScaleDefinitions.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QGroupBox>
#include <QComboBox>
#include <QTimer>
#include <QPushButton>
#include <QDebug>
#include <QMetaObject>

// Helper from main.cpp, now local to where it's used
std::vector<std::string> getChordNotes(const QString& chordName);

MainWindow::MainWindow(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    noteMapFretboard = new NoteMapFretboardWidget(this);

    // --- Metronome Group ---
    QGroupBox* metronomeGroup = new QGroupBox("Metronome & Timing", this);
    QVBoxLayout* metronomeLayout = new QVBoxLayout();

    // BPM controls
    QHBoxLayout *bpmLayout = new QHBoxLayout();
    QLabel *bpmLabel = new QLabel("BPM:", this);
    bpmValue = new QLabel(QString::number(metronome.getBpm()), this);
    slider = new QSlider(Qt::Horizontal, this);
    slider->setRange(40, 240);
    slider->setValue(metronome.getBpm());
    connect(slider, &QSlider::valueChanged, this, &MainWindow::onBPMChanged);
    bpmLayout->addWidget(bpmLabel);
    bpmLayout->addWidget(slider);
    bpmLayout->addWidget(bpmValue);
    metronomeLayout->addLayout(bpmLayout);

    // Time signature controls
    QHBoxLayout *beatsLayout = new QHBoxLayout();
    QLabel *beatsLabel = new QLabel("Beats per bar:", this);
    beatsValue = new QLabel(QString::number(metronome.getBeatsPerBar()), this);
    beatsSlider = new QSlider(Qt::Horizontal, this);
    beatsSlider->setRange(1, 12);
    beatsSlider->setValue(metronome.getBeatsPerBar());
    connect(beatsSlider, &QSlider::valueChanged, this, &MainWindow::onBeatsChanged);
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
    connect(denomCombo, &QComboBox::currentTextChanged, this, &MainWindow::onDenomChanged);
    beatsLayout->addWidget(denomLabel);
    beatsLayout->addWidget(denomCombo);
    metronomeLayout->addLayout(beatsLayout);

    // Subdivision controls
    QHBoxLayout *subdivLayout = new QHBoxLayout();
    QLabel *subdivLabel = new QLabel("Subdivisions per beat:", this);
    subdivValue = new QLabel(QString::number(metronome.getSubdivisions()), this);
    subdivSlider = new QSlider(Qt::Horizontal, this);
    subdivSlider->setRange(1, 8);
    subdivSlider->setValue(metronome.getSubdivisions());
    connect(subdivSlider, &QSlider::valueChanged, this, &MainWindow::onSubdivChanged);
    subdivLayout->addWidget(subdivLabel);
    subdivLayout->addWidget(subdivSlider);
    subdivLayout->addWidget(subdivValue);
    metronomeLayout->addLayout(subdivLayout);

    // Volume controls
    QHBoxLayout *volumeLayout = new QHBoxLayout();
    QLabel *volumeLabel = new QLabel("Metronome Volume:", this);
    volumeValue = new QLabel(QString::number(metronome.getVolume(), 'f', 2), this);
    volumeSlider = new QSlider(Qt::Horizontal, this);
    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(static_cast<int>(metronome.getVolume() * 100));
    connect(volumeSlider, &QSlider::valueChanged, this, &MainWindow::onVolumeChanged);
    volumeLayout->addWidget(volumeLabel);
    volumeLayout->addWidget(volumeSlider);
    volumeLayout->addWidget(volumeValue);
    metronomeLayout->addLayout(volumeLayout);

    // Synth selection
    QHBoxLayout* synthLayout = new QHBoxLayout();
    QLabel* synthLabel = new QLabel("Synth:", this);
    synthCombo = new QComboBox(this);
    synthCombo->addItem("Guitar (Karplus-Strong)");
    synthCombo->addItem("Subtractive");
    synthCombo->addItem("FM");
    synthCombo->addItem("Wavetable");
    synthCombo->addItem("Karplus-Strong (Custom)");
    connect(synthCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
        ChordAudioEngine::SynthType synthType = ChordAudioEngine::SynthType::Guitar;
        if (idx == 1) synthType = ChordAudioEngine::SynthType::Subtractive;
        else if (idx == 2) synthType = ChordAudioEngine::SynthType::FM;
        else if (idx == 3) synthType = ChordAudioEngine::SynthType::Wavetable;
        else if (idx == 4) synthType = ChordAudioEngine::SynthType::KarplusStrong;
        audioEngine.setSynthType(synthType);
        if (synthSettingsWindow) {
            synthSettingsWindow->setCurrentSynthType(synthType);
        }
    });
    synthLayout->addWidget(synthLabel);
    synthLayout->addWidget(synthCombo);
    metronomeLayout->addLayout(synthLayout);

    metronomeGroup->setLayout(metronomeLayout);
    layout->addWidget(metronomeGroup);

    // --- Effects Window ---
    effectsWindow = new EffectsWindow();
    QPushButton* showEffectsButton = new QPushButton("Show Effects Rack", this);
    connect(showEffectsButton, &QPushButton::clicked, effectsWindow, &EffectsWindow::show);
    layout->addWidget(showEffectsButton);

    QPushButton* toggleScaleButton = new QPushButton("Scale Overlay: ON", this);
    toggleScaleButton->setCheckable(true);
    toggleScaleButton->setChecked(true);
    connect(toggleScaleButton, &QPushButton::toggled, this, [this, toggleScaleButton](bool enabled) {
        showScaleOverlay_ = enabled;
        toggleScaleButton->setText(enabled ? "Scale Overlay: ON" : "Scale Overlay: OFF");
        refreshFretboardDisplay();
    });
    layout->addWidget(toggleScaleButton);

    QPushButton* toggleFxButton = new QPushButton("FX Chain: ON", this);
    toggleFxButton->setCheckable(true);
    toggleFxButton->setChecked(true);
    connect(toggleFxButton, &QPushButton::toggled, this, [this, toggleFxButton](bool enabled) {
        audioEngine.setFXEnabled(enabled);
        toggleFxButton->setText(enabled ? "FX Chain: ON" : "FX Chain: OFF");
    });
    layout->addWidget(toggleFxButton);

    synthSettingsWindow = new SynthSettingsWindow(&audioEngine, this);
    synthSettingsWindow->setCurrentSynthType(ChordAudioEngine::SynthType::Guitar);
    QPushButton* showSynthSettingsButton = new QPushButton("Show Synth Settings", this);
    connect(showSynthSettingsButton, &QPushButton::clicked, this, [this]() {
        synthSettingsWindow->show();
        synthSettingsWindow->raise();
        synthSettingsWindow->activateWindow();
    });
    layout->addWidget(showSynthSettingsButton);

    // Connect signals from the new effects window to the main window's slots
    connect(effectsWindow, &EffectsWindow::reverbMixChanged, this, &MainWindow::onReverbMixChanged);
    connect(effectsWindow, &EffectsWindow::delayTimeChanged, this, &MainWindow::onDelayTimeChanged);
    connect(effectsWindow, &EffectsWindow::delayFeedbackChanged, this, &MainWindow::onDelayFeedbackChanged);
    connect(effectsWindow, &EffectsWindow::delayMixChanged, this, &MainWindow::onDelayMixChanged);
    connect(effectsWindow, &EffectsWindow::distortionDriveChanged, this, &MainWindow::onDistortionDriveChanged);
    connect(effectsWindow, &EffectsWindow::distortionMixChanged, this, &MainWindow::onDistortionMixChanged);
    connect(effectsWindow, &EffectsWindow::compressorThresholdChanged, this, &MainWindow::onCompressorThresholdChanged);
    connect(effectsWindow, &EffectsWindow::compressorRatioChanged, this, &MainWindow::onCompressorRatioChanged);
    connect(effectsWindow, &EffectsWindow::compressorAttackChanged, this, &MainWindow::onCompressorAttackChanged);
    connect(effectsWindow, &EffectsWindow::compressorReleaseChanged, this, &MainWindow::onCompressorReleaseChanged);
    connect(effectsWindow, &EffectsWindow::compressorKneeChanged, this, &MainWindow::onCompressorKneeChanged);
    connect(effectsWindow, &EffectsWindow::compressorSaturationChanged, this, &MainWindow::onCompressorSaturationChanged);
    connect(effectsWindow, &EffectsWindow::compressorDetectorBlendChanged, this, &MainWindow::onCompressorDetectorBlendChanged);
    connect(effectsWindow, &EffectsWindow::eqLowGainChanged, this, &MainWindow::onEQLowGainChanged);
    connect(effectsWindow, &EffectsWindow::eqMidGainChanged, this, &MainWindow::onEQMidGainChanged);
    connect(effectsWindow, &EffectsWindow::eqHighGainChanged, this, &MainWindow::onEQHighGainChanged);
    connect(effectsWindow, &EffectsWindow::eqSaturationEnabledChanged, this, &MainWindow::onEQSaturationEnabledChanged);
    connect(effectsWindow, &EffectsWindow::eqSaturationDriveChanged, this, &MainWindow::onEQSaturationDriveChanged);
    connect(effectsWindow, &EffectsWindow::eqMSAmountChanged, this, &MainWindow::onEQMSAmountChanged);
    connect(effectsWindow, &EffectsWindow::limiterThresholdChanged, this, &MainWindow::onLimiterThresholdChanged);
    connect(effectsWindow, &EffectsWindow::limiterReleaseChanged, this, &MainWindow::onLimiterReleaseChanged);
    connect(effectsWindow, &EffectsWindow::chorusRateChanged, this, &MainWindow::onChorusRateChanged);
    connect(effectsWindow, &EffectsWindow::chorusDepthChanged, this, &MainWindow::onChorusDepthChanged);
    connect(effectsWindow, &EffectsWindow::chorusMixChanged, this, &MainWindow::onChorusMixChanged);
    connect(effectsWindow, &EffectsWindow::gateThresholdChanged, this, &MainWindow::onGateThresholdChanged);
    connect(effectsWindow, &EffectsWindow::gateReleaseChanged, this, &MainWindow::onGateReleaseChanged);
    connect(effectsWindow, &EffectsWindow::flangerRateChanged, this, &MainWindow::onFlangerRateChanged);
    connect(effectsWindow, &EffectsWindow::flangerDepthChanged, this, &MainWindow::onFlangerDepthChanged);
    connect(effectsWindow, &EffectsWindow::flangerFeedbackChanged, this, &MainWindow::onFlangerFeedbackChanged);
    connect(effectsWindow, &EffectsWindow::flangerMixChanged, this, &MainWindow::onFlangerMixChanged);
    connect(effectsWindow, &EffectsWindow::phaserRateChanged, this, &MainWindow::onPhaserRateChanged);
    connect(effectsWindow, &EffectsWindow::phaserDepthChanged, this, &MainWindow::onPhaserDepthChanged);
    connect(effectsWindow, &EffectsWindow::phaserFeedbackChanged, this, &MainWindow::onPhaserFeedbackChanged);
    connect(effectsWindow, &EffectsWindow::phaserMixChanged, this, &MainWindow::onPhaserMixChanged);
    connect(effectsWindow, &EffectsWindow::tremoloRateChanged, this, &MainWindow::onTremoloRateChanged);
    connect(effectsWindow, &EffectsWindow::tremoloDepthChanged, this, &MainWindow::onTremoloDepthChanged);
    // Note: Tremolo mix is not connected as it's not implemented in the UI separately
    connect(effectsWindow, &EffectsWindow::orderChanged, this, [this](const QStringList& newOrder) {
        std::vector<std::string> order;
        for (const QString& s : newOrder) {
            order.push_back(s.toStdString());
        }
        audioEngine.setFXOrder(order);
    });
    connect(effectsWindow, &EffectsWindow::fxChainEnabledChanged, this, [this](bool enabled) {
        audioEngine.setFXEnabled(enabled);
    });

    // Chord progression widget
    progressionWidget = new ChordProgressionWidget(this);
    layout->addWidget(progressionWidget);
    connect(progressionWidget, &ChordProgressionWidget::playRequested, this, &MainWindow::onPlayRequested);
    connect(progressionWidget, &ChordProgressionWidget::stopRequested, this, [this]() {
        audioEngine.stop();
        metronome.reset();
        currentChordName_.clear();
        onScaleSelected(progressionWidget->scaleRoot(), progressionWidget->scaleType()); // Re-apply scale display on stop
    });
    connect(progressionWidget, &ChordProgressionWidget::chordVolumeChanged, this, [this](double vol){ audioEngine.setVolume(vol); });
    connect(progressionWidget, &ChordProgressionWidget::scaleChanged, this, &MainWindow::onScaleSelected);

    // Audio engine callback to update fretboard
    audioEngine.setChordChangeCallback([this](const std::string& chordName) {
        QMetaObject::invokeMethod(this, "onChordSelected", Qt::QueuedConnection, Q_ARG(QString, QString::fromStdString(chordName)));
    });

    // Add the note map fretboard to the layout
    layout->addWidget(noteMapFretboard);

    legendWidget = new ColorLegendsWith(this);
    layout->addWidget(legendWidget);

    setLayout(layout);
    setWindowTitle("Metronome & Backing Track");
    resize(900, 800);

    // Initialize metronome with UI values
    metronome.setBpm(slider->value());
    metronome.setBeatsPerBar(beatsSlider->value());
    metronome.setSubdivisions(subdivSlider->value());
    metronome.setVolume(volumeSlider->value() / 100.0);

    // Set initial effect values
    float initialReverb = 0.3f;
    float initialDelayTime = 500.0f;
    float initialDelayFeedback = 0.5f;
    float initialDelayMix = 0.3f;
    effectsWindow->setReverbMix(initialReverb);
    effectsWindow->setDelayTime(initialDelayTime);
    effectsWindow->setDelayFeedback(initialDelayFeedback);
    effectsWindow->setDelayMix(initialDelayMix);
    effectsWindow->setFXOrder({"Delay", "Reverb"});

    onScaleSelected(progressionWidget->scaleRoot(), progressionWidget->scaleType()); // Initial scale display

    // Set up a timer to poll for meter updates
    meterUpdateTimer_ = new QTimer(this);
    connect(meterUpdateTimer_, &QTimer::timeout, this, &MainWindow::onUpdateMeters);
    meterUpdateTimer_->start(33); // Update roughly 30 times per second
}

void MainWindow::onPlayRequested(const std::vector<ChordChange>& progression) {
    audioEngine.stop();
    ChordProgression prog;
    for (const auto& c : progression) {
        prog.addChord(c.chordName, c.bars, c.strummingPatternIndex);
    }
    audioEngine.setProgression(prog);
    audioEngine.setMetronome(&metronome);
    audioEngine.setBpm(metronome.getBpm());
    audioEngine.setSampleRate(48000);
    audioEngine.start();
}

void MainWindow::onBPMChanged(int value) {
    metronome.setBpm(value);
    bpmValue->setText(QString::number(value));
}

void MainWindow::onBeatsChanged(int value) {
    metronome.setBeatsPerBar(value);
    beatsValue->setText(QString::number(value));
}

void MainWindow::onDenomChanged(const QString &text) {
    timeSigDenominator = text.toInt();
    metronome.setTimeSignatureDenominator(timeSigDenominator);
}

void MainWindow::onSubdivChanged(int value) {
    metronome.setSubdivisions(value);
    subdivValue->setText(QString::number(value));
}

void MainWindow::onVolumeChanged(int value) {
    double vol = value / 100.0;
    metronome.setVolume(vol);
    volumeValue->setText(QString::number(vol, 'f', 2));
}

void MainWindow::onReverbMixChanged(int value) {
    audioEngine.setReverbMix(value / 100.0f);
}

void MainWindow::onDelayTimeChanged(int value) {
    audioEngine.setDelayTime(static_cast<float>(value));
}

void MainWindow::onDelayFeedbackChanged(int value) {
    audioEngine.setDelayFeedback(value / 100.0f);
}

void MainWindow::onDelayMixChanged(int value) {
    audioEngine.setDelayMix(value / 100.0f);
}

void MainWindow::onDistortionDriveChanged(int value) {
    audioEngine.setDistortionDrive(value / 100.0f);
}

void MainWindow::onDistortionMixChanged(int value) {
    audioEngine.setDistortionMix(value / 100.0f);
}

void MainWindow::onCompressorThresholdChanged(int value) {
    audioEngine.setCompressorThreshold(static_cast<float>(value));
}

void MainWindow::onCompressorRatioChanged(int value) {
    audioEngine.setCompressorRatio(static_cast<float>(value));
}

void MainWindow::onCompressorAttackChanged(int value) {
    audioEngine.setCompressorAttack(static_cast<float>(value));
}

void MainWindow::onCompressorReleaseChanged(int value) {
    audioEngine.setCompressorRelease(static_cast<float>(value));
}

void MainWindow::onCompressorKneeChanged(int value) {
    audioEngine.setCompressorKnee(static_cast<float>(value));
}

void MainWindow::onCompressorSaturationChanged(int value) {
    audioEngine.setCompressorSaturationDrive(static_cast<float>(value) / 10.0f);
}

void MainWindow::onCompressorDetectorBlendChanged(int value) {
    audioEngine.setCompressorDetectorBlend(static_cast<float>(value) / 100.0f);
}

void MainWindow::onEQLowGainChanged(int value) {
    audioEngine.setEQLowGain(static_cast<float>(value));
}

void MainWindow::onEQMidGainChanged(int value) {
    audioEngine.setEQMidGain(static_cast<float>(value));
}

void MainWindow::onEQHighGainChanged(int value) {
    audioEngine.setEQHighGain(static_cast<float>(value));
}

void MainWindow::onEQSaturationEnabledChanged(bool enabled) {
    audioEngine.setEQSaturationEnabled(enabled);
}

void MainWindow::onEQSaturationDriveChanged(int value) {
    audioEngine.setEQSaturationDrive(static_cast<float>(value) / 10.0f);
}

void MainWindow::onEQMSAmountChanged(int value) {
    audioEngine.setEQMSAmount(static_cast<float>(value) / 100.0f);
}

void MainWindow::onLimiterThresholdChanged(int value) {
    audioEngine.setLimiterThreshold(static_cast<float>(value) / 10.0f);
}

void MainWindow::onLimiterReleaseChanged(int value) {
    audioEngine.setLimiterRelease(static_cast<float>(value));
}

void MainWindow::onChorusRateChanged(int value) {
    audioEngine.setChorusRate(static_cast<float>(value) / 10.0f);
}

void MainWindow::onChorusDepthChanged(int value) {
    audioEngine.setChorusDepth(static_cast<float>(value) / 10.0f);
}

void MainWindow::onChorusMixChanged(int value) {
    audioEngine.setChorusMix(static_cast<float>(value) / 100.0f);
}

void MainWindow::onGateThresholdChanged(int value) {
    audioEngine.setGateThreshold(static_cast<float>(value));
}

void MainWindow::onGateReleaseChanged(int value) {
    audioEngine.setGateRelease(static_cast<float>(value));
}

void MainWindow::onFlangerRateChanged(int value) {
    audioEngine.setFlangerRate(static_cast<float>(value) / 10.0f);
}

void MainWindow::onFlangerDepthChanged(int value) {
    audioEngine.setFlangerDepth(static_cast<float>(value) / 10.0f);
}

void MainWindow::onFlangerFeedbackChanged(int value) {
    audioEngine.setFlangerFeedback(static_cast<float>(value) / 100.0f);
}

void MainWindow::onFlangerMixChanged(int value) {
    audioEngine.setFlangerMix(static_cast<float>(value) / 100.0f);
}

void MainWindow::onPhaserRateChanged(int value) {
    audioEngine.setPhaserRate(static_cast<float>(value) / 10.0f);
}

void MainWindow::onPhaserDepthChanged(int value) {
    audioEngine.setPhaserDepth(static_cast<float>(value) / 100.0f);
}

void MainWindow::onPhaserFeedbackChanged(int value) {
    audioEngine.setPhaserFeedback(static_cast<float>(value) / 100.0f);
}

void MainWindow::onPhaserMixChanged(int value) {
    audioEngine.setPhaserMix(static_cast<float>(value) / 100.0f);
}

void MainWindow::onTremoloRateChanged(int value) {
    audioEngine.setTremoloRate(static_cast<float>(value) / 10.0f);
}

void MainWindow::onTremoloDepthChanged(int value) {
    audioEngine.setTremoloDepth(static_cast<float>(value) / 100.0f);
}

void MainWindow::onUpdateMeters() {
    effectsWindow->updateCompressorMeter(audioEngine.getCompressorGainReductionDB());
    effectsWindow->updateSpectrum(audioEngine.getSpectrumData());
}

void MainWindow::refreshFretboardDisplay() {
    if (currentChordName_.empty()) {
        if (!audioEngine.isRunning()) {
            if (showScaleOverlay_ && !currentScaleNotes_.empty()) {
                NoteMap noteMap = makeNoteMap(currentScaleNotes_, currentScaleLabel_);
                noteMapFretboard->setNoteMap(noteMap);
            } else {
                noteMapFretboard->setNoteMap(NoteMap());
            }
        }
        return;
    }

    std::vector<std::string> chordNotes = getChordNotes(QString::fromStdString(currentChordName_));
    std::vector<std::string> allNotesToDisplay = chordNotes;
    if (showScaleOverlay_) {
        allNotesToDisplay = currentScaleNotes_;
        for (const auto& chordNote : chordNotes) {
            if (std::find(allNotesToDisplay.begin(), allNotesToDisplay.end(), chordNote) == allNotesToDisplay.end()) {
                allNotesToDisplay.push_back(chordNote);
            }
        }
    }

    std::string displayLabel = currentChordName_;
    if (showScaleOverlay_ && !currentScaleLabel_.empty()) {
        displayLabel += " in " + currentScaleLabel_;
    }

    NoteMap noteMap = makeNoteMap(allNotesToDisplay, displayLabel);
    noteMap.chordTones = chordNotes;
    noteMapFretboard->setNoteMap(noteMap);
}

void MainWindow::onChordSelected(const QString& chordName) {
    currentChordName_ = chordName.toStdString();
    refreshFretboardDisplay();
}

void MainWindow::onScaleSelected(const QString& scaleRoot, const QString& scaleName) {
    currentScaleNotes_.clear();
    currentScaleLabel_.clear();
    if (scaleRoot.isEmpty() || scaleName.isEmpty()) {
        if (!audioEngine.isRunning()) noteMapFretboard->setNoteMap(NoteMap());
        return;
    }

    static const QMap<QString, int> noteIndices = {
        {"C",0}, {"C#",1}, {"Db",1}, {"D",2}, {"D#",3}, {"Eb",3}, {"E",4}, {"F",5},
        {"F#",6}, {"Gb",6}, {"G",7}, {"G#",8}, {"Ab",8}, {"A",9}, {"A#",10}, {"Bb",10}, {"B",11}
    };

    int rootIdx = noteIndices.value(scaleRoot, 0);
    const auto& scaleDef = kScaleDefinitions.at(scaleName.toStdString());
    currentScaleLabel_ = (scaleRoot + " " + scaleName).toStdString();

    int currentNote = rootIdx;
    currentScaleNotes_.push_back(noteIndices.key(currentNote).toStdString());
    for (int interval : scaleDef.intervals) {
        currentNote = (currentNote + interval) % 12;
        currentScaleNotes_.push_back(noteIndices.key(currentNote).toStdString());
    }

    refreshFretboardDisplay();
}