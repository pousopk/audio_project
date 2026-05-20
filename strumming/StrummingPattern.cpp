#include "StrummingPattern.h"

const std::vector<StrummingPattern> kStrummingPatterns = {
    {"Full Bar Down", {{0.0, StrumType::Down, 0}}},
    {"Half Down Half Up", {{0.0, StrumType::Down, 0}, {0.5, StrumType::Up, 0}}},
    {"Down on 1", {{0.0, StrumType::Down, 0}}},
    {"Down-Up", {{0.0, StrumType::Down, 0}, {0.5, StrumType::Up, 0}}},
    {"Arpeggio Asc 4", {{0.0, StrumType::ArpeggioAsc, 4}}},
    {"Arpeggio Desc 4", {{0.0, StrumType::ArpeggioDesc, 4}}}
};
