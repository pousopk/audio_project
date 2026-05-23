#pragma once
#include <map>
#include <string>
#include <vector>
#include "ChordChange.h"

/**
 * @brief Manages chord progressions, including saving/loading and standard progressions.
 */
class ChordProgressionManager {
public:
    /** @brief Construct a ChordProgressionManager. */
    ChordProgressionManager();

    /**
     * @brief Add a chord to the progression.
     * @param chordName Name of the chord.
     * @param bars Number of bars to play.
     * @param strummingPatternIndex Strumming pattern index.
     */
    void addChord(const std::string& chordName, int bars, int strummingPatternIndex = 0);
    /** @brief Remove a chord at the given index. */
    void removeChord(int index);
    /** @brief Clear the progression. */
    void clear();
    /** @brief Get the current progression. */
    std::vector<ChordChange> getProgression() const;
    /** @brief Save the progression by name. */
    void saveProgression(const std::string& name);
    /** @brief Load a progression by name. */
    void loadProgression(const std::string& name);
    /** @brief List all saved progressions. */
    std::vector<std::string> listSavedProgressions() const;
    /** @brief Set a standard progression (e.g., I-IV-V). */
    void setStandardProgression(const std::string& standardName, int strummingPatternIndex);
    /** @brief Get all available standard progression names. */
    std::vector<std::string> standardProgressionNames() const;

private:
    std::vector<ChordChange> progression_;
    std::map<std::string, std::vector<ChordChange>> savedProgressions_;
    std::map<std::string, std::vector<ChordChange>> standardProgressions_;
    void initStandardProgressions();
};
