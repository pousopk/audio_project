#pragma once
#include <QWidget>
#include <vector>
#include <QString>

/**
 * @brief Maps notes to fret positions for a chord or scale.
 */
struct NoteMap {
    std::vector<std::vector<int>> noteFrets; ///< For each string, fret numbers where target notes occur
    QString chordName; ///< Name of the chord
    std::vector<QString> chordTones; ///< Notes to highlight as target tones
};

/**
 * @brief Widget for displaying a fretboard with highlighted notes.
 */
class NoteMapFretboardWidget : public QWidget {
    Q_OBJECT
public:
    /** @brief Construct a NoteMapFretboardWidget. */
    explicit NoteMapFretboardWidget(QWidget* parent = nullptr);
    /** @brief Set the note map to display. */
    void setNoteMap(const NoteMap& map);
protected:
    /** @brief Paint the fretboard and note highlights. */
    void paintEvent(QPaintEvent* event) override;
private:
    NoteMap currentMap;
    int numFrets = 24;
    int numStrings = 6;
};
