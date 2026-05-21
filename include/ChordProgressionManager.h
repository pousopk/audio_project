#pragma once
#include <QString>
#include <QVector>
#include <QMap>


/**
 * @brief Represents a chord change in a progression.
 */
struct ChordChange {
    QString chordName; ///< Name of the chord
    int bars; ///< Number of bars to play this chord
    int strummingPatternIndex = 0; ///< Strumming pattern index
};

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
    void addChord(const QString& chordName, int bars, int strummingPatternIndex = 0);
    /** @brief Remove a chord at the given index. */
    void removeChord(int index);
    /** @brief Clear the progression. */
    void clear();
    /** @brief Get the current progression. */
    QVector<ChordChange> getProgression() const;
    /** @brief Save the progression by name. */
    void saveProgression(const QString& name);
    /** @brief Load a progression by name. */
    void loadProgression(const QString& name);
    /** @brief List all saved progressions. */
    QStringList listSavedProgressions() const;
    /** @brief Set a standard progression (e.g., I-IV-V). */
    void setStandardProgression(const QString& standardName, int strummingPatternIndex);
    /** @brief Get all available standard progression names. */
    QStringList standardProgressionNames() const;

private:
    QVector<ChordChange> progression_;
    QMap<QString, QVector<ChordChange>> savedProgressions_;
    QMap<QString, QVector<ChordChange>> standardProgressions_;
    void initStandardProgressions();
};
