#pragma once
#include <QString>
#include <QVector>
#include <QMap>

struct ChordChange {
    QString chordName;
    int bars; // Number of bars to play this chord
    int strummingPatternIndex = 0;
};

class ChordProgressionManager {
public:
    ChordProgressionManager();

    // Add a chord to the progression
    void addChord(const QString& chordName, int bars, int strummingPatternIndex = 0);
    // Remove a chord at index
    void removeChord(int index);
    // Clear the progression
    void clear();
    // Get the current progression
    QVector<ChordChange> getProgression() const;
    // Save/load progression by name
    void saveProgression(const QString& name);
    void loadProgression(const QString& name);
    // List all saved progressions
    QStringList listSavedProgressions() const;
    // Set a standard progression (e.g., I-IV-V)
    void setStandardProgression(const QString& standardName, int strummingPatternIndex);
    // Get all available standard progressions
    QStringList standardProgressionNames() const;

private:
    QVector<ChordChange> progression_;
    QMap<QString, QVector<ChordChange>> savedProgressions_;
    QMap<QString, QVector<ChordChange>> standardProgressions_;
    void initStandardProgressions();
};
