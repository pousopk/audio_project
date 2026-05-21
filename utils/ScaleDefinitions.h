#pragma once

#include <string>
#include <vector>
#include <map>

struct ScaleDefinition {
    std::string name;
    std::vector<int> intervals; // in semitones
    std::vector<std::string> chordTypes; // diatonic chord types for a 7-note scale (e.g., "", "m", "m", "", "7", "m", "dim")
};

extern const std::map<std::string, ScaleDefinition> kScaleDefinitions;