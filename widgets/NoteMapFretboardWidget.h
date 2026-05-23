#pragma once
#include <QWidget>
#include "../include/NoteMap.h"

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
