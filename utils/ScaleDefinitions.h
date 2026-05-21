#pragma once

#include <string>
#include <vector>
#include <map>

/**
 * @brief Definition of a musical scale.
 */
struct ScaleDefinition {
    std::string name; ///< Name of the scale
    std::vector<int> intervals; ///< Intervals in semitones
    std::vector<std::string> chordTypes; ///< Diatonic chord types for a 7-note scale
};

/**
 * @brief Map of scale names to their definitions.
 */
extern const std::map<std::string, ScaleDefinition> kScaleDefinitions;