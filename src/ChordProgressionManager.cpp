#include "ChordProgressionManager.h"

ChordProgressionManager::ChordProgressionManager() {
    initStandardProgressions();
}

void ChordProgressionManager::addChord(const QString& chordName, int bars, int strummingPatternIndex) {
    progression_.append({chordName, bars, strummingPatternIndex});
}

void ChordProgressionManager::removeChord(int index) {
    if (index >= 0 && index < progression_.size())
        progression_.removeAt(index);
}

void ChordProgressionManager::clear() {
    progression_.clear();
}

QVector<ChordChange> ChordProgressionManager::getProgression() const {
    return progression_;
}

void ChordProgressionManager::saveProgression(const QString& name) {
    savedProgressions_[name] = progression_;
}

void ChordProgressionManager::loadProgression(const QString& name) {
    if (savedProgressions_.contains(name))
        progression_ = savedProgressions_[name];
}

QStringList ChordProgressionManager::listSavedProgressions() const {
    return savedProgressions_.keys();
}

void ChordProgressionManager::setStandardProgression(const QString& standardName) {
    if (standardProgressions_.contains(standardName))
        progression_ = standardProgressions_[standardName];
}

QStringList ChordProgressionManager::standardProgressionNames() const {
    return standardProgressions_.keys();
}

void ChordProgressionManager::initStandardProgressions() {
    // I-IV-V in C
    standardProgressions_["I-IV-V (C)"] = {
        {"C", 2}, {"F", 2}, {"G", 4}, {"C", 2}
    };
    // 12-bar blues in E
    standardProgressions_["12-bar Blues (E)"] = {
        {"E", 4}, {"A", 2}, {"E", 2}, {"B", 1}, {"A", 1}, {"E", 2}
    };
    // Pop I-V-vi-IV in G
    standardProgressions_["I-V-vi-IV (G)"] = {
        {"G", 2}, {"D", 2}, {"Em", 2}, {"C", 2}
    };
}
