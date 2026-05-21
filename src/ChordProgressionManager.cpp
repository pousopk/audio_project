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

void ChordProgressionManager::setStandardProgression(const QString& standardName, int strummingPatternIndex) {
    if (standardProgressions_.contains(standardName)) {
        progression_ = standardProgressions_[standardName];
        for (auto& chord : progression_) {
            chord.strummingPatternIndex = strummingPatternIndex;
        }
    }
}

QStringList ChordProgressionManager::standardProgressionNames() const {
    return standardProgressions_.keys();
}

void ChordProgressionManager::initStandardProgressions() {
    // I-IV-V in C
    standardProgressions_["Pop: I-V-vi-IV (C)"] = {
        {"C", 2}, {"G", 2}, {"Am", 2}, {"F", 2}
    };
    // 12-bar blues in E
    standardProgressions_["12-bar Blues (E)"] = {
        {"E", 4}, {"A", 2}, {"E", 2}, {"B", 1}, {"A", 1}, {"E", 2}
    };
    // Pop I-V-vi-IV in G
    standardProgressions_["Pop: I-V-vi-IV (G)"] = {
        {"G", 2}, {"D", 2}, {"Em", 2}, {"C", 2}
    };
    // Jazz: ii-V-I in C
    standardProgressions_["Jazz: ii-V-I (C)"] = {
        {"Dm7", 2}, {"G7", 2}, {"Cmaj7", 4}
    };
    // Minor: i-VI-III-VII in Am
    standardProgressions_["Minor: i-VI-III-VII (Am)"] = {
        {"Am", 2}, {"F", 2}, {"C", 2}, {"G", 2}
    };
    // 50s: I-vi-IV-V in C
    standardProgressions_["50s: I-vi-IV-V (C)"] = {
        {"C", 2}, {"Am", 2}, {"F", 2}, {"G", 2}
    };
    // Pachelbel's Canon in D
    standardProgressions_["Canon: I-V-vi-iii-IV-I-IV-V (D)"] = {
        {"D", 2}, {"A", 2}, {"Bm", 2}, {"F#m", 2},
        {"G", 2}, {"D", 2}, {"G", 2}, {"A", 2}
    };
    // Jazz standard "Autumn Leaves" in Em
    standardProgressions_["Jazz: Autumn Leaves (Em)"] = {
        {"Am7", 2}, {"D7", 2}, {"Gmaj7", 2}, {"Cmaj7", 2},
        {"F#m7b5", 2}, {"B7", 2}, {"Em", 4}
    };
    // Jazz Blues in Bb
    standardProgressions_["Jazz Blues (Bb)"] = {
        {"Bb7", 1}, {"Eb7", 1}, {"Bb7", 2},
        {"Eb7", 2}, {"Edim7", 2},
        {"Bb7", 1}, {"G7", 1}, {"Cm7", 1}, {"F7", 1},
        {"Bb7", 1}, {"F7", 1}
    };
    // Classic Rock Ballad intro
    standardProgressions_["Rock: Chromatic Ballad (Am)"] = {
        {"Am", 1}, {"G#aug", 1}, {"C", 1}, {"D", 1}, {"Fmaj7", 2}, {"G", 1}, {"Am", 1}
    };
    // Bluegrass: "Will the Circle Be Unbroken" in G
    standardProgressions_["Bluegrass: Circle Be Unbroken (G)"] = {
        {"G", 4}, {"C", 2}, {"G", 2}, {"D7", 2}, {"G", 2}
    };
    // Bluegrass: "I'll Fly Away" in G
    standardProgressions_["Bluegrass: I'll Fly Away (G)"] = {
        {"G", 4}, {"C", 2}, {"G", 2}, {"G", 1}, {"D7", 1}, {"G", 2}
    };
    // Bluegrass: "Nine Pound Hammer" in G
    standardProgressions_["Bluegrass: Nine Pound Hammer (G)"] = {
        {"G", 2}, {"C", 2}, {"G", 2}, {"D", 2}
    };
    // Bluegrass: "Cripple Creek" in G
    standardProgressions_["Bluegrass: Cripple Creek (G)"] = {
        {"G", 8}, {"C", 2}, {"G", 2}, {"D", 2}, {"G", 2}
    };
    // Bluegrass: "Bury Me Beneath the Willow" in G
    standardProgressions_["Bluegrass: Bury Me Beneath the Willow (G)"] = {
        {"G", 4}, {"C", 2}, {"G", 2}, {"D", 2}, {"G", 2}
    };
    // Bluegrass: "Long Journey Home" in G
    standardProgressions_["Bluegrass: Long Journey Home (G)"] = {
        {"G", 4}, {"C", 2}, {"G", 1}, {"D", 1}, {"G", 2}
    };
    // Bluegrass: "Salt Creek" in A
    standardProgressions_["Bluegrass: Salt Creek (A)"] = {
        {"A", 2}, {"G", 2}, {"A", 2}, {"G", 1}, {"E", 1}
    };

}
