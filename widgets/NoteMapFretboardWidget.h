#pragma once
#include <QWidget>
#include <vector>
#include <QString>

struct NoteMap {
    // For each string, a vector of fret numbers where the target notes occur
    std::vector<std::vector<int>> noteFrets; // size = numStrings
    QString chordName;
    std::vector<QString> chordTones; // Notes to highlight as "target tones"
};

class NoteMapFretboardWidget : public QWidget {
    Q_OBJECT
public:
    explicit NoteMapFretboardWidget(QWidget* parent = nullptr);
    void setNoteMap(const NoteMap& map);
protected:
    void paintEvent(QPaintEvent* event) override;
private:
    NoteMap currentMap;
    int numFrets = 24;
    int numStrings = 6;
};
