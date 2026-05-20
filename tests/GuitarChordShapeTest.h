#pragma once
#include <vector>
#include <string>

// Qt-free version for tests
struct GuitarChordShapeTest {
    std::vector<int> frets; // size = 6 for standard guitar
    std::vector<int> fingers; // optional: finger numbers (0 = none)
    std::string name;
};
