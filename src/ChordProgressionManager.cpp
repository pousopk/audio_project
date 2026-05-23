#include "ChordProgressionManager.h"
#include <algorithm>

ChordProgressionManager::ChordProgressionManager() {
    initStandardProgressions();
}

/**
 * @brief Add a chord to the current progression.
 * @param chordName Name of the chord.
 * @param bars Number of bars for the chord.
 * @param strummingPatternIndex Index of the strumming pattern.
 */
void ChordProgressionManager::addChord(const std::string& chordName, int bars, int strummingPatternIndex) {
    progression_.push_back({chordName, bars, strummingPatternIndex});
}

/**
 * @brief Remove a chord from the progression by index.
 * @param index Index of the chord to remove.
 */
void ChordProgressionManager::removeChord(int index) {
    if (index >= 0 && static_cast<std::size_t>(index) < progression_.size()) {
        progression_.erase(progression_.begin() + index);
    }
}

/**
 * @brief Clear the current chord progression.
 */
void ChordProgressionManager::clear() {
    progression_.clear();
}

/**
 * @brief Get the current chord progression.
 * @return Vector of ChordChange.
 */
std::vector<ChordChange> ChordProgressionManager::getProgression() const {
    return progression_;
}

/**
 * @brief Save the current progression under a given name.
 * @param name Name to save the progression as.
 */
void ChordProgressionManager::saveProgression(const std::string& name) {
    savedProgressions_[name] = progression_;
}

/**
 * @brief Load a saved progression by name.
 * @param name Name of the saved progression.
 */
void ChordProgressionManager::loadProgression(const std::string& name) {
    auto it = savedProgressions_.find(name);
    if (it != savedProgressions_.end()) {
        progression_ = it->second;
    }
}

/**
 * @brief List all saved progression names.
 * @return Vector of saved progression names.
 */
std::vector<std::string> ChordProgressionManager::listSavedProgressions() const {
    std::vector<std::string> names;
    names.reserve(savedProgressions_.size());
    for (const auto& pair : savedProgressions_) {
        names.push_back(pair.first);
    }
    return names;
}

/**
 * @brief Set the progression to a standard pattern and apply a strumming pattern index.
 * @param standardName Name of the standard progression.
 * @param strummingPatternIndex Index of the strumming pattern.
 */
void ChordProgressionManager::setStandardProgression(const std::string& standardName, int strummingPatternIndex) {
    auto it = standardProgressions_.find(standardName);
    if (it != standardProgressions_.end()) {
        progression_ = it->second;
        for (auto& chord : progression_) {
            chord.strummingPatternIndex = strummingPatternIndex;
        }
    }
}

/**
 * @brief Get the names of all standard progressions.
 * @return Vector of standard progression names.
 */
std::vector<std::string> ChordProgressionManager::standardProgressionNames() const {
    std::vector<std::string> names;
    names.reserve(standardProgressions_.size());
    for (const auto& pair : standardProgressions_) {
        names.push_back(pair.first);
    }
    return names;
}

/**
 * @brief Initialize the set of standard chord progressions.
 */
void ChordProgressionManager::initStandardProgressions() {
    // I-IV-V in C
    standardProgressions_["Pop: I-V-vi-IV (C)"] = {
        {"C", 2}, {"G", 2}, {"Am", 2}, {"F", 2}
    };
    // 12-bar blues in E
    standardProgressions_["12-bar Blues (E)"] = {
        {"E", 4}, {"A", 2}, {"E", 2}, {"B7", 1}, {"A", 1}, {"E", 2}
    };
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
