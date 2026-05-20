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
#include "../include/ChordNames.h"

ChordProgressionWidget::ChordProgressionWidget(QWidget* parent)
    : QWidget(parent)
{

    chordList_ = new QListWidget(this);
    rootCombo_ = new QComboBox(this);
    typeCombo_ = new QComboBox(this);

    barsEdit_ = new QLineEdit(this);

    // Populate root and type combos
    QStringList roots = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    QStringList types = {"", "m", "7", "m7", "maj7"};
    rootCombo_->addItems(roots);
    typeCombo_->addItems(types);

    // Strumming pattern combo
    strumCombo_ = new QComboBox(this);
    extern const std::vector<StrummingPattern> kStrummingPatterns;
    for (const auto& pattern : kStrummingPatterns) {
        strumCombo_->addItem(QString::fromStdString(pattern.name));
    }
    addButton_ = new QPushButton("Add Chord", this);
    removeButton_ = new QPushButton("Remove Selected", this);
    saveButton_ = new QPushButton("Save Progression", this);
    loadButton_ = new QPushButton("Load Progression", this);
    savedCombo_ = new QComboBox(this);
    standardCombo_ = new QComboBox(this);
    appendStandardButton_ = new QPushButton("Append Standard", this);
    playButton_ = new QPushButton("Play", this);
    stopButton_ = new QPushButton("Stop", this);

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
    manageLayout->addWidget(saveButton_);
    manageLayout->addWidget(new QLabel("Saved:"));
    manageLayout->addWidget(savedCombo_);
    manageLayout->addWidget(loadButton_);

    QHBoxLayout* standardLayout = new QHBoxLayout;
    standardLayout->addWidget(new QLabel("Standard:"));
    standardLayout->addWidget(standardCombo_);
    standardLayout->addWidget(appendStandardButton_);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(addLayout);
    mainLayout->addWidget(chordList_);
    mainLayout->addLayout(manageLayout);
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
    connect(appendStandardButton_, &QPushButton::clicked, this, &ChordProgressionWidget::appendStandardToList);

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

void ChordProgressionWidget::addChord() {
    QString root = rootCombo_->currentText().trimmed();
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

void ChordProgressionWidget::appendStandardToList() {
    QString name = standardCombo_->currentText();
    if (!name.isEmpty()) {
        auto orig = manager_.getProgression();
        manager_.setStandardProgression(name);
        auto stdprog = manager_.getProgression();
        manager_.clear();
        for (const auto& c : orig) manager_.addChord(c.chordName, c.bars);
        for (const auto& c : stdprog) manager_.addChord(c.chordName, c.bars);
        updateProgressionList();
        emit progressionChanged(manager_.getProgression());
    }
}

void ChordProgressionWidget::playProgression() {
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
        manager_.setStandardProgression(name);
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
