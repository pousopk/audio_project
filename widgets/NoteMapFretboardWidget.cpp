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
    // For degree coloring, parse chord notes from chordName if possible (e.g. "E major note map (E, G#, B)")
    std::vector<QString> chordNotes;
    int l = currentMap.chordName.indexOf('(');
    int r = currentMap.chordName.indexOf(')');
    if (l != -1 && r != -1 && r > l) {
        QString notesStr = currentMap.chordName.mid(l+1, r-l-1);
        QStringList noteList = notesStr.split(',', Qt::SkipEmptyParts);
        for (QString n : noteList) chordNotes.push_back(n.trimmed());
    }
    // Draw note positions (top = high E)
    QFont font = p.font();
    font.setPointSize(9);
    p.setFont(font);
    for (int s = 0; s < currentMap.noteFrets.size(); ++s) {
        int y = marginY + (numStrings - 1 - s) * stringSpacing;
        for (int fret : currentMap.noteFrets[s]) {
            if (fret >= 0 && fret <= numFrets) {
                int x = marginX + fret * fretSpacing;
                // Determine note name for this position
                QString noteName;
                // Standard tuning MIDI for each string
                static const int stringMidi[6] = {40, 45, 50, 55, 59, 64};
                int midi = stringMidi[s] + fret;
                int noteClass = midi % 12;
                static const char* noteNames[12] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
                noteName = noteNames[noteClass];
                // Get degree and color
                int degree = -1;
                QColor color = Qt::gray;
                if (!chordNotes.empty()) {
                    auto dc = getNoteDegreeAndColor(noteName, chordNotes);
                    degree = dc.first;
                    color = dc.second;
                }
                p.setBrush(color);
                p.setPen(Qt::black);
                p.drawEllipse(QPoint(x, y), 10, 10);
                // Draw degree number (1=root, 3=3rd, 5=5th, 7=7th)
                if (degree >= 0 && degree <= 3) {
                    QString num = (degree == 0) ? "1" : (degree == 1) ? "3" : (degree == 2) ? "5" : "7";
                    p.setPen(Qt::white);
                    p.drawText(x-6, y+5, num);
                } else {
                    p.setPen(Qt::white);
                    p.drawText(x-6, y+5, noteName);
                }
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
