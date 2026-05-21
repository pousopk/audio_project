#include "ChordProgressionWidget.h"
#include "../strumming/StrummingPattern.h"
#include <QListWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QInputDialog>
#include <QStyle>
#include "../include/ChordNames.h"
#include "../utils/ScaleDefinitions.h"

ChordProgressionWidget::ChordProgressionWidget(QWidget* parent)
    : QWidget(parent)
{

    chordList_ = new QListWidget(this);
    rootCombo_ = new QComboBox(this);
    typeCombo_ = new QComboBox(this);

    barsEdit_ = new QLineEdit(this);

    // Populate root and type combos
    QStringList roots = {"C", "C#", "Db", "D", "D#", "Eb", "E", "F", "F#", "Gb", "G", "G#", "Ab", "A", "A#", "Bb", "B"};
    QStringList types = {"", "m", "7", "m7", "maj7", "sus2", "sus4", "dim", "aug", "add9", "6", "m7b5", "dim7", "7#9", "7b9"};
    rootCombo_->addItems(roots);
    typeCombo_->addItems(types);

    // Strumming pattern combo
    strumCombo_ = new QComboBox(this);
    extern const std::vector<StrummingPattern> kStrummingPatterns;
    for (const auto& pattern : kStrummingPatterns) {
        strumCombo_->addItem(QString::fromStdString(pattern.name));
    }
    addButton_ = new QPushButton(this);
    addButton_->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
    addButton_->setToolTip("Add Chord");
    removeButton_ = new QPushButton(this);
    removeButton_->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
    removeButton_->setToolTip("Remove Selected");
    saveButton_ = new QPushButton(this);
    saveButton_->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    saveButton_->setToolTip("Save Progression");
    loadButton_ = new QPushButton(this);
    loadButton_->setIcon(style()->standardIcon(QStyle::SP_DialogOkButton));
    loadButton_->setToolTip("Load Selected Progression");
    savedCombo_ = new QComboBox(this);
    standardCombo_ = new QComboBox(this);
    playButton_ = new QPushButton(this);
    playButton_->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    playButton_->setToolTip("Play");
    stopButton_ = new QPushButton(this);
    stopButton_->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    stopButton_->setToolTip("Stop");

    // Scale selection UI
    scaleRootCombo_ = new QComboBox(this);
    scaleRootCombo_->addItems(roots);
    scaleTypeCombo_ = new QComboBox(this);
    for (const auto& pair : kScaleDefinitions) {
        scaleTypeCombo_->addItem(QString::fromStdString(pair.first));
    }
    connect(scaleRootCombo_, &QComboBox::currentTextChanged, this, [this](){ emit scaleChanged(scaleRootCombo_->currentText(), scaleTypeCombo_->currentText()); });
    connect(scaleTypeCombo_, &QComboBox::currentTextChanged, this, [this](){ emit scaleChanged(scaleRootCombo_->currentText(), scaleTypeCombo_->currentText()); });

    QHBoxLayout* addLayout = new QHBoxLayout;
    addLayout->addWidget(new QLabel("Root:"));
    addLayout->addWidget(rootCombo_);
    addLayout->addWidget(new QLabel("Type:"));
    addLayout->addWidget(typeCombo_);
    addLayout->addWidget(new QLabel("Bars:"));
    addLayout->addWidget(barsEdit_);
    addLayout->addWidget(new QLabel("Strum:"));
    addLayout->addWidget(strumCombo_);
    addLayout->addWidget(addButton_);
    addLayout->addWidget(playButton_);
    addLayout->addWidget(stopButton_);

    QHBoxLayout* manageLayout = new QHBoxLayout;
    manageLayout->addWidget(removeButton_);
    manageLayout->addWidget(new QLabel("Saved:"));
    manageLayout->addWidget(savedCombo_);
    manageLayout->addWidget(loadButton_);
    manageLayout->addWidget(saveButton_);

    QHBoxLayout* standardLayout = new QHBoxLayout; // This layout might be removed or repurposed
    standardLayout->addWidget(new QLabel("Templates:"));
    standardLayout->addWidget(standardCombo_);

    QHBoxLayout* scaleLayout = new QHBoxLayout;
    scaleLayout->addWidget(new QLabel("Highlight Scale:"));
    scaleLayout->addWidget(scaleRootCombo_);
    scaleLayout->addWidget(scaleTypeCombo_);

    // Chord Volume
    QHBoxLayout* chordVolumeLayout = new QHBoxLayout;
    chordVolumeSlider_ = new QSlider(Qt::Horizontal, this);
    chordVolumeSlider_->setRange(0, 100);
    chordVolumeSlider_->setValue(100);
    chordVolumeValue_ = new QLabel("1.00", this);
    connect(chordVolumeSlider_, &QSlider::valueChanged, this, [this](int value){
        double vol = value / 100.0;
        emit chordVolumeChanged(vol);
        chordVolumeValue_->setText(QString::number(vol, 'f', 2));
    });
    chordVolumeLayout->addWidget(new QLabel("Chord Volume:"));
    chordVolumeLayout->addWidget(chordVolumeSlider_);
    chordVolumeLayout->addWidget(chordVolumeValue_);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(addLayout);
    mainLayout->addWidget(chordList_);
    mainLayout->addLayout(chordVolumeLayout);
    mainLayout->addLayout(manageLayout);
    mainLayout->addLayout(scaleLayout);
    mainLayout->addLayout(standardLayout);
    setLayout(mainLayout);

    refreshSavedCombo();
    refreshStandardCombo();
    updateProgressionList();

    connect(addButton_, &QPushButton::clicked, this, &ChordProgressionWidget::addChord);
    connect(removeButton_, &QPushButton::clicked, this, &ChordProgressionWidget::removeSelectedChord);
    connect(saveButton_, &QPushButton::clicked, this, &ChordProgressionWidget::saveProgression);
    connect(loadButton_, &QPushButton::clicked, this, &ChordProgressionWidget::loadProgression);
    connect(standardCombo_, &QComboBox::currentTextChanged, this, &ChordProgressionWidget::setStandardProgression);

    connect(playButton_, &QPushButton::clicked, this, &ChordProgressionWidget::playProgression);
    connect(stopButton_, &QPushButton::clicked, this, &ChordProgressionWidget::stopProgression);
}

void ChordProgressionWidget::stopProgression() {
    // Do not clear progression or UI fields, just stop audio
    emit stopRequested();
}

ChordProgressionManager* ChordProgressionWidget::progressionManager() {
    return &manager_;
}

QString ChordProgressionWidget::scaleRoot() const {
    return scaleRootCombo_->currentText();
}

QString ChordProgressionWidget::scaleType() const {
    return scaleTypeCombo_->currentText();
}

void ChordProgressionWidget::addChord() {
    QString root = rootCombo_->currentText().trimmed();
    root.remove('*'); // Remove the suggestion marker before creating the chord
    QString type = typeCombo_->currentText().trimmed();
    QString chord = root + type;
    bool ok;
    int bars = barsEdit_->text().toInt(&ok);
    if (root.isEmpty() || !ok || bars < 1) {
        QMessageBox::warning(this, "Input Error", "Enter a valid chord and number of bars.");
        return;
    }
    int strumIdx = strumCombo_ ? strumCombo_->currentIndex() : 0;
    // If ChordProgressionManager::addChord only takes 2 args, use that
    manager_.addChord(chord, bars, strumIdx);
    updateProgressionList();
    emit progressionChanged(manager_.getProgression());
}

void ChordProgressionWidget::playProgression() {
    // Set initial volume when play is pressed
    emit chordVolumeChanged(chordVolumeSlider_->value() / 100.0);
    emit playRequested(manager_.getProgression());
}

void ChordProgressionWidget::removeSelectedChord() {
    int row = chordList_->currentRow();
    if (row >= 0) {
        manager_.removeChord(row);
        updateProgressionList();
        emit progressionChanged(manager_.getProgression());
    }
}

void ChordProgressionWidget::saveProgression() {
    bool ok;
    QString name = QInputDialog::getText(this, "Save Progression", "Name:", QLineEdit::Normal, "", &ok);
    if (ok && !name.trimmed().isEmpty()) {
        manager_.saveProgression(name.trimmed());
        refreshSavedCombo();
    }
}

void ChordProgressionWidget::loadProgression() {
    QString name = savedCombo_->currentText();
    if (!name.isEmpty()) {
        manager_.loadProgression(name);
        updateProgressionList();
        emit progressionChanged(manager_.getProgression());
    }
}

void ChordProgressionWidget::setStandardProgression() {
    QString name = standardCombo_->currentText();
    if (!name.isEmpty()) {
        int strumIdx = strumCombo_ ? strumCombo_->currentIndex() : 0;
        manager_.setStandardProgression(name, strumIdx);
        updateProgressionList();
        emit progressionChanged(manager_.getProgression());
    }
}

void ChordProgressionWidget::updateProgressionList() {
    chordList_->clear();
    for (const auto& change : manager_.getProgression()) {
        chordList_->addItem(QString("%1 (%2 bars)").arg(change.chordName).arg(change.bars));
    }
}

void ChordProgressionWidget::refreshSavedCombo() {
    savedCombo_->clear();
    savedCombo_->addItems(manager_.listSavedProgressions());
}

void ChordProgressionWidget::refreshStandardCombo() {
    standardCombo_->clear();
    standardCombo_->addItems(manager_.standardProgressionNames());
}
