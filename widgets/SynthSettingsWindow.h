#pragma once

#include <QWidget>
#include <vector>
#include "ChordAudioEngine.h"

class QSlider;
class QComboBox;

class SynthSettingsWindow : public QWidget {
public:
    explicit SynthSettingsWindow(ChordAudioEngine* engine, QWidget* parent = nullptr);
    void setCurrentSynthType(ChordAudioEngine::SynthType type);

private:
    void updateVisibleGroups();
    void applyKarplusPresetFromUi(int index);
    void setKarplusCustomEditingEnabled(bool enabled);
    void syncKarplusSlidersToPreset(ChordAudioEngine::KarplusPreset preset);

    ChordAudioEngine* engine_ = nullptr;
    ChordAudioEngine::SynthType currentSynthType_ = ChordAudioEngine::SynthType::Guitar;

    QWidget* guitarGroup_ = nullptr;
    QWidget* subtractiveGroup_ = nullptr;
    QWidget* fmGroup_ = nullptr;
    QWidget* wavetableGroup_ = nullptr;
    QWidget* karplusCustomGroup_ = nullptr;

    QComboBox* karplusPresetCombo_ = nullptr;
    std::vector<QSlider*> karplusCustomSliders_;
    bool applyingKarplusUiState_ = false;
};
