#pragma once
#include <vector>
#include <QString>
// Simple chord shape: -1 = muted, 0 = open, 1+ = fret number
struct GuitarChordShape {
    std::vector<int> frets; // size = 6 for standard guitar
    std::vector<int> fingers; // optional: finger numbers (0 = none)
    QString name;
};
