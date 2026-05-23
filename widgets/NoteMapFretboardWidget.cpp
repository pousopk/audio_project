#include "NoteMapFretboardWidget.h"
#include "NoteColorUtils.h"
#include <QPainter>
#include <QFont>
#include <QStringList>

NoteMapFretboardWidget::NoteMapFretboardWidget(QWidget* parent)
    : QWidget(parent) {
    setMinimumSize(900, 250);
}

void NoteMapFretboardWidget::setNoteMap(const NoteMap& map) {
    currentMap = map;
    update();
}

void NoteMapFretboardWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    int w = width();
    int h = height();
    int marginX = 40;
    int marginY = 40;
    int fretSpacing = (w - 2 * marginX) / numFrets;
    int stringSpacing = (h - 2 * marginY) / (numStrings - 1);

    // Draw frets (vertical lines)
    for (int f = 0; f <= numFrets; ++f) {
        int x = marginX + f * fretSpacing;
        p.drawLine(x, marginY, x, h - marginY);
        if (f > 0 && f <= numFrets) {
            p.drawText(x - fretSpacing / 2, marginY - 10, QString::number(f));
        }
    }
    // Draw strings (horizontal lines, top = high E)
    for (int s = 0; s < numStrings; ++s) {
        int y = marginY + (numStrings - 1 - s) * stringSpacing;
        p.drawLine(marginX, y, w - marginX, y);
    }
    // Determine the root note from the chord/scale name (e.g., "C#m7" -> "C#")
    QString rootNote = currentMap.chordName.left(1);
    if (currentMap.chordName.length() > 1 && (currentMap.chordName[1] == '#' || currentMap.chordName[1] == 'b')) {
        rootNote = currentMap.chordName.left(2);
    }
    // Draw note positions (top = high E)
    QFont font = p.font();
    font.setPointSize(9);
    p.setFont(font);
    for (std::size_t s = 0; s < currentMap.noteFrets.size(); ++s) {
        const int stringIndex = static_cast<int>(s);
        int y = marginY + (numStrings - 1 - stringIndex) * stringSpacing;
        for (int fret : currentMap.noteFrets[s]) {
            if (fret >= 0 && fret <= numFrets) {
                int x;
                if (fret == 0) {
                    x = marginX - fretSpacing / 2; // Position for open strings
                } else {
                    x = marginX + fret * fretSpacing - fretSpacing / 2; // Position between frets
                }
                // Determine note name for this position
                QString noteName;
                // Standard tuning MIDI for each string
                static const int stringMidi[6] = {40, 45, 50, 55, 59, 64};
                int midi = stringMidi[stringIndex] + fret;
                int noteClass = midi % 12;
                static const char* noteNames[12] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
                noteName = noteNames[noteClass];
                // Get degree and color
                auto intervalAndColor = getIntervalAndColor(noteName, rootNote);
                QString intervalText = intervalAndColor.first;
                QColor color = intervalAndColor.second;

                // Check if this note is a "target tone" (a chord tone)
                bool isTargetTone = std::find(currentMap.chordTones.begin(), currentMap.chordTones.end(), noteName) != currentMap.chordTones.end();

                // Draw highlight for target tones
                if (isTargetTone) {
                    p.setPen(QPen(QColor(255, 255, 0, 200), 3)); // Bright yellow outline
                    p.drawEllipse(QPoint(x, y), 12, 12);
                }

                p.setBrush(color);
                p.setPen(Qt::black);
                p.drawEllipse(QPoint(x, y), 10, 10);

                // Draw interval text inside the circle
                p.setPen(Qt::white);
                font.setPointSize(7);
                p.setFont(font);
                p.drawText(QRect(x-10, y-10, 20, 20), Qt::AlignCenter, intervalText);
            }
        }
    }
    // Draw chord name
    if (!currentMap.chordName.isEmpty()) {
        QFont font = p.font();
        font.setPointSize(14);
        font.setBold(true);
        p.setFont(font);
        p.drawText(rect(), Qt::AlignTop | Qt::AlignHCenter, currentMap.chordName);
    }
}
