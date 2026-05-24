#include "SynthSettingsWindow.h"

#include "ChordAudioEngine.h"

#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QVBoxLayout>

#include <functional>

namespace {
struct KarplusPresetValues {
    int decay;
    int brightness;
    int transient;
    int dispersion;
    int drive;
    int body;
    int sympathetic;
};

KarplusPresetValues valuesForPreset(ChordAudioEngine::KarplusPreset preset) {
    switch (preset) {
        case ChordAudioEngine::KarplusPreset::Warm:
            return {9940, 10, 12, 4, 11, 75, 5};
        case ChordAudioEngine::KarplusPreset::Bright:
            return {9890, 26, 35, 14, 20, 40, 2};
        case ChordAudioEngine::KarplusPreset::Muted:
            return {9780, 8, 6, 2, 9, 35, 0};
        case ChordAudioEngine::KarplusPreset::Custom:
        case ChordAudioEngine::KarplusPreset::Balanced:
        default:
            return {9920, 14, 20, 8, 13, 55, 3};
    }
}

ChordAudioEngine::KarplusPreset presetFromIndex(int idx) {
    switch (idx) {
        case 1: return ChordAudioEngine::KarplusPreset::Warm;
        case 2: return ChordAudioEngine::KarplusPreset::Bright;
        case 3: return ChordAudioEngine::KarplusPreset::Muted;
        case 4: return ChordAudioEngine::KarplusPreset::Custom;
        default: return ChordAudioEngine::KarplusPreset::Balanced;
    }
}
} // namespace

SynthSettingsWindow::SynthSettingsWindow(ChordAudioEngine* engine, QWidget* parent)
    : QWidget(parent, Qt::Window), engine_(engine) {
    setWindowTitle("Synth Settings");
    setMinimumWidth(520);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    auto addSlider = [this](QVBoxLayout* layout,
                            const QString& label,
                            int min,
                            int max,
                            int initial,
                            const std::function<QString(int)>& formatter,
                            const std::function<void(int)>& onChange) -> QSlider* {
        QHBoxLayout* row = new QHBoxLayout();
        row->addWidget(new QLabel(label, this));

        QSlider* slider = new QSlider(Qt::Horizontal, this);
        slider->setRange(min, max);

        QLabel* value = new QLabel(formatter(initial), this);

        connect(slider, &QSlider::valueChanged, this, [value, formatter, onChange](int v) {
            value->setText(formatter(v));
            onChange(v);
        });

        row->addWidget(slider, 1);
        row->addWidget(value);
        layout->addLayout(row);

        slider->setValue(initial);
        return slider;
    };

    // Guitar synth
    {
        QGroupBox* group = new QGroupBox("Guitar (Karplus-Strong)", this);
        guitarGroup_ = group;
        QVBoxLayout* layout = new QVBoxLayout();

        addSlider(layout, "Brightness", 0, 100, 70,
                  [](int v) { return QString::number(v / 100.0, 'f', 2); },
                  [this](int v) {
                      if (engine_) engine_->setGuitarFilterCoefficient(v / 100.0);
                  });

        addSlider(layout, "Decay", 900, 999, 990,
                  [](int v) { return QString::number(v / 1000.0, 'f', 3); },
                  [this](int v) {
                      if (engine_) engine_->setGuitarDecayFactor(v / 1000.0);
                  });

        group->setLayout(layout);
        mainLayout->addWidget(group);
    }

    // Subtractive synth
    {
        QGroupBox* group = new QGroupBox("Subtractive", this);
        subtractiveGroup_ = group;
        QVBoxLayout* layout = new QVBoxLayout();

        QHBoxLayout* waveformRow = new QHBoxLayout();
        waveformRow->addWidget(new QLabel("Waveform", this));
        QComboBox* waveform = new QComboBox(this);
        waveform->addItem("Saw");
        waveform->addItem("Square");
        connect(waveform, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
            if (engine_) engine_->setSubtractiveWaveformSaw(idx == 0);
        });
        waveformRow->addWidget(waveform);
        layout->addLayout(waveformRow);

        addSlider(layout, "Cutoff (Hz)", 80, 12000, 2000,
                  [](int v) { return QString::number(v); },
                  [this](int v) {
                      if (engine_) engine_->setSubtractiveCutoff(static_cast<double>(v));
                  });

        QSlider* subAttack = addSlider(layout, "Attack (ms)", 1, 2000, 10,
                                       [](int v) { return QString::number(v); },
                                       [](int) {});
        QSlider* subDecay = addSlider(layout, "Decay (ms)", 1, 2000, 50,
                                      [](int v) { return QString::number(v); },
                                      [](int) {});
        QSlider* subSustain = addSlider(layout, "Sustain", 0, 100, 70,
                                        [](int v) { return QString::number(v / 100.0, 'f', 2); },
                                        [](int) {});
        QSlider* subRelease = addSlider(layout, "Release (ms)", 1, 4000, 100,
                                        [](int v) { return QString::number(v); },
                                        [](int) {});

        auto applySubEnv = [this, subAttack, subDecay, subSustain, subRelease]() {
            if (!engine_) return;
            engine_->setSubtractiveEnvelope(subAttack->value() / 1000.0,
                                            subDecay->value() / 1000.0,
                                            subSustain->value() / 100.0,
                                            subRelease->value() / 1000.0);
        };
        connect(subAttack, &QSlider::valueChanged, this, [applySubEnv](int) { applySubEnv(); });
        connect(subDecay, &QSlider::valueChanged, this, [applySubEnv](int) { applySubEnv(); });
        connect(subSustain, &QSlider::valueChanged, this, [applySubEnv](int) { applySubEnv(); });
        connect(subRelease, &QSlider::valueChanged, this, [applySubEnv](int) { applySubEnv(); });
        applySubEnv();

        group->setLayout(layout);
        mainLayout->addWidget(group);
    }

    // FM synth
    {
        QGroupBox* group = new QGroupBox("FM", this);
        fmGroup_ = group;
        QVBoxLayout* layout = new QVBoxLayout();

        addSlider(layout, "Mod Ratio", 1, 120, 20,
                  [](int v) { return QString::number(v / 10.0, 'f', 1); },
                  [this](int v) {
                      if (engine_) engine_->setFMModRatio(v / 10.0);
                  });

        addSlider(layout, "Mod Index", 0, 200, 30,
                  [](int v) { return QString::number(v / 10.0, 'f', 1); },
                  [this](int v) {
                      if (engine_) engine_->setFMModIndex(v / 10.0);
                  });

        QSlider* fmAttack = addSlider(layout, "Attack (ms)", 1, 2000, 10,
                                      [](int v) { return QString::number(v); },
                                      [](int) {});
        QSlider* fmDecay = addSlider(layout, "Decay (ms)", 1, 2000, 50,
                                     [](int v) { return QString::number(v); },
                                     [](int) {});
        QSlider* fmSustain = addSlider(layout, "Sustain", 0, 100, 70,
                                       [](int v) { return QString::number(v / 100.0, 'f', 2); },
                                       [](int) {});
        QSlider* fmRelease = addSlider(layout, "Release (ms)", 1, 4000, 100,
                                       [](int v) { return QString::number(v); },
                                       [](int) {});

        auto applyFmEnv = [this, fmAttack, fmDecay, fmSustain, fmRelease]() {
            if (!engine_) return;
            engine_->setFMEnvelope(fmAttack->value() / 1000.0,
                                   fmDecay->value() / 1000.0,
                                   fmSustain->value() / 100.0,
                                   fmRelease->value() / 1000.0);
        };
        connect(fmAttack, &QSlider::valueChanged, this, [applyFmEnv](int) { applyFmEnv(); });
        connect(fmDecay, &QSlider::valueChanged, this, [applyFmEnv](int) { applyFmEnv(); });
        connect(fmSustain, &QSlider::valueChanged, this, [applyFmEnv](int) { applyFmEnv(); });
        connect(fmRelease, &QSlider::valueChanged, this, [applyFmEnv](int) { applyFmEnv(); });
        applyFmEnv();

        group->setLayout(layout);
        mainLayout->addWidget(group);
    }

    // Wavetable synth
    {
        QGroupBox* group = new QGroupBox("Wavetable", this);
        wavetableGroup_ = group;
        QVBoxLayout* layout = new QVBoxLayout();

        QSlider* wtAttack = addSlider(layout, "Attack (ms)", 1, 2000, 10,
                                      [](int v) { return QString::number(v); },
                                      [](int) {});
        QSlider* wtDecay = addSlider(layout, "Decay (ms)", 1, 2000, 50,
                                     [](int v) { return QString::number(v); },
                                     [](int) {});
        QSlider* wtSustain = addSlider(layout, "Sustain", 0, 100, 70,
                                       [](int v) { return QString::number(v / 100.0, 'f', 2); },
                                       [](int) {});
        QSlider* wtRelease = addSlider(layout, "Release (ms)", 1, 4000, 100,
                                       [](int v) { return QString::number(v); },
                                       [](int) {});

        auto applyWtEnv = [this, wtAttack, wtDecay, wtSustain, wtRelease]() {
            if (!engine_) return;
            engine_->setWavetableEnvelope(wtAttack->value() / 1000.0,
                                          wtDecay->value() / 1000.0,
                                          wtSustain->value() / 100.0,
                                          wtRelease->value() / 1000.0);
        };
        connect(wtAttack, &QSlider::valueChanged, this, [applyWtEnv](int) { applyWtEnv(); });
        connect(wtDecay, &QSlider::valueChanged, this, [applyWtEnv](int) { applyWtEnv(); });
        connect(wtSustain, &QSlider::valueChanged, this, [applyWtEnv](int) { applyWtEnv(); });
        connect(wtRelease, &QSlider::valueChanged, this, [applyWtEnv](int) { applyWtEnv(); });
        applyWtEnv();

        group->setLayout(layout);
        mainLayout->addWidget(group);
    }

    // Custom Karplus-Strong
    {
        QGroupBox* group = new QGroupBox("Karplus-Strong (Custom)", this);
        karplusCustomGroup_ = group;
        QVBoxLayout* layout = new QVBoxLayout();

        QHBoxLayout* presetRow = new QHBoxLayout();
        presetRow->addWidget(new QLabel("Preset", this));
        karplusPresetCombo_ = new QComboBox(this);
        karplusPresetCombo_->addItem("Balanced");
        karplusPresetCombo_->addItem("Warm");
        karplusPresetCombo_->addItem("Bright");
        karplusPresetCombo_->addItem("Muted");
        karplusPresetCombo_->addItem("Custom");
        presetRow->addWidget(karplusPresetCombo_, 1);
        layout->addLayout(presetRow);

        auto markCustomIfEditing = [this]() {
            if (applyingKarplusUiState_ || !karplusPresetCombo_) return;
            if (karplusPresetCombo_->currentIndex() != 4) {
                karplusPresetCombo_->setCurrentIndex(4);
            }
        };

        QSlider* decaySlider = addSlider(layout, "Decay", 9000, 9999, 9920,
                  [](int v) { return QString::number(v / 10000.0, 'f', 4); },
                  [this, markCustomIfEditing](int v) {
                      if (applyingKarplusUiState_) return;
                      if (engine_) engine_->setKarplusCustomDecay(v / 10000.0);
                      markCustomIfEditing();
                  });
        karplusCustomSliders_.push_back(decaySlider);

        QSlider* brightnessSlider = addSlider(layout, "Exciter Brightness", 2, 95, 14,
                  [](int v) { return QString::number(v / 100.0, 'f', 2); },
                  [this, markCustomIfEditing](int v) {
                      if (applyingKarplusUiState_) return;
                      if (engine_) engine_->setKarplusExciterBrightness(v / 100.0);
                      markCustomIfEditing();
                  });
        karplusCustomSliders_.push_back(brightnessSlider);

        QSlider* transientSlider = addSlider(layout, "Pick Transient", 0, 150, 20,
                  [](int v) { return QString::number(v / 100.0, 'f', 2); },
                  [this, markCustomIfEditing](int v) {
                      if (applyingKarplusUiState_) return;
                      if (engine_) engine_->setKarplusPickTransient(v / 100.0);
                      markCustomIfEditing();
                  });
        karplusCustomSliders_.push_back(transientSlider);

        QSlider* dispersionSlider = addSlider(layout, "Dispersion", 0, 85, 8,
                  [](int v) { return QString::number(v / 100.0, 'f', 2); },
                  [this, markCustomIfEditing](int v) {
                      if (applyingKarplusUiState_) return;
                      if (engine_) engine_->setKarplusDispersion(v / 100.0);
                      markCustomIfEditing();
                  });
        karplusCustomSliders_.push_back(dispersionSlider);

        QSlider* driveSlider = addSlider(layout, "String Drive", 2, 80, 8,
                  [](int v) { return QString::number(v / 10.0, 'f', 1); },
                  [this, markCustomIfEditing](int v) {
                      if (applyingKarplusUiState_) return;
                      if (engine_) engine_->setKarplusDrive(v / 10.0);
                      markCustomIfEditing();
                  });
        karplusCustomSliders_.push_back(driveSlider);

        QSlider* bodySlider = addSlider(layout, "Body Resonance", 0, 300, 55,
                  [](int v) { return QString::number(v / 100.0, 'f', 2); },
                  [this, markCustomIfEditing](int v) {
                      if (applyingKarplusUiState_) return;
                      if (engine_) engine_->setKarplusBodyResonance(v / 100.0);
                      markCustomIfEditing();
                  });
        karplusCustomSliders_.push_back(bodySlider);

        QSlider* sympatheticSlider = addSlider(layout, "Sympathetic", 0, 60, 3,
                  [](int v) { return QString::number(v / 100.0, 'f', 2); },
                  [this, markCustomIfEditing](int v) {
                      if (applyingKarplusUiState_) return;
                      if (engine_) engine_->setKarplusSympatheticResonance(v / 100.0);
                      markCustomIfEditing();
                  });
        karplusCustomSliders_.push_back(sympatheticSlider);

        connect(karplusPresetCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, [this](int idx) { applyKarplusPresetFromUi(idx); });

        karplusPresetCombo_->setCurrentIndex(0);
        applyKarplusPresetFromUi(0);

        group->setLayout(layout);
        mainLayout->addWidget(group);
    }

    mainLayout->addStretch();
    setLayout(mainLayout);
    updateVisibleGroups();
}

void SynthSettingsWindow::setCurrentSynthType(ChordAudioEngine::SynthType type) {
    currentSynthType_ = type;
    updateVisibleGroups();
}

void SynthSettingsWindow::updateVisibleGroups() {
    if (!guitarGroup_ || !subtractiveGroup_ || !fmGroup_ || !wavetableGroup_ || !karplusCustomGroup_) {
        return;
    }

    guitarGroup_->setVisible(currentSynthType_ == ChordAudioEngine::SynthType::Guitar);
    subtractiveGroup_->setVisible(currentSynthType_ == ChordAudioEngine::SynthType::Subtractive);
    fmGroup_->setVisible(currentSynthType_ == ChordAudioEngine::SynthType::FM);
    wavetableGroup_->setVisible(currentSynthType_ == ChordAudioEngine::SynthType::Wavetable);
    karplusCustomGroup_->setVisible(currentSynthType_ == ChordAudioEngine::SynthType::KarplusStrong);
}

void SynthSettingsWindow::setKarplusCustomEditingEnabled(bool enabled) {
    for (QSlider* slider : karplusCustomSliders_) {
        if (slider) slider->setEnabled(enabled);
    }
}

void SynthSettingsWindow::syncKarplusSlidersToPreset(ChordAudioEngine::KarplusPreset preset) {
    if (karplusCustomSliders_.size() < 7) return;
    const KarplusPresetValues p = valuesForPreset(preset);
    const int values[7] = {p.decay, p.brightness, p.transient, p.dispersion, p.drive, p.body, p.sympathetic};

    applyingKarplusUiState_ = true;
    for (size_t i = 0; i < karplusCustomSliders_.size() && i < 7; ++i) {
        if (!karplusCustomSliders_[i]) continue;
        karplusCustomSliders_[i]->setValue(values[i]);
    }
    applyingKarplusUiState_ = false;
}

void SynthSettingsWindow::applyKarplusPresetFromUi(int index) {
    if (!engine_) return;

    const ChordAudioEngine::KarplusPreset preset = presetFromIndex(index);
    if (preset == ChordAudioEngine::KarplusPreset::Custom) {
        engine_->applyKarplusPreset(ChordAudioEngine::KarplusPreset::Custom);
        setKarplusCustomEditingEnabled(true);
        return;
    }

    engine_->applyKarplusPreset(preset);
    syncKarplusSlidersToPreset(preset);
    setKarplusCustomEditingEnabled(false);
}
