#include "ScaleDefinitions.h"

const std::map<std::string, ScaleDefinition> kScaleDefinitions = {
    {"Major", {"Major",
        {2, 2, 1, 2, 2, 2, 1}, // W-W-H-W-W-W-H
        {"", "m", "m", "", "7", "m", "dim"}
    }},
    {"Natural Minor", {"Natural Minor",
        {2, 1, 2, 2, 1, 2, 2}, // W-H-W-W-H-W-W
        {"m", "dim", "", "m", "m", "", "7"}
    }},
    {"Major Pentatonic", {"Major Pentatonic",
        {2, 2, 3, 2, 3},
        {} // No standard diatonic triads for pentatonic
    }},
    {"Dorian", {"Dorian",
        {2, 1, 2, 2, 2, 1, 2}, // W-H-W-W-W-H-W
        {"m", "m", "", "7", "m", "dim", ""}
    }},
    {"Phrygian", {"Phrygian",
        {1, 2, 2, 2, 1, 2, 2}, // H-W-W-W-H-W-W
        {"m", "", "7", "m", "dim", "", "m"}
    }},
    {"Lydian", {"Lydian",
        {2, 2, 2, 1, 2, 2, 1}, // W-W-W-H-W-W-H
        {"", "7", "m", "dim", "", "m", "m"}
    }},
    {"Mixo-lydian", {"Mixo-lydian",
        {2, 2, 1, 2, 2, 1, 2}, // W-W-H-W-W-H-W
        {"7", "m", "dim", "", "m", "m", ""}
    }},
    {"Locrian", {"Locrian",
        {1, 2, 2, 1, 2, 2, 2}, // H-W-W-H-W-W-W
        {"dim", "", "m", "m", "", "7", "m"}
    }}
};