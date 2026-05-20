#include "HorizontalFretboardWidget.h"
#include <QPainter>
#include <QFont>
#include <algorithm>
#include "../utils/NoteColorUtils.h"

HorizontalFretboardWidget::HorizontalFretboardWidget(QWidget* parent)
    : QWidget(parent) {
    setMinimumSize(900, 250);
}

void HorizontalFretboardWidget::setChordInversions(const std::vector<GuitarChordShape>& inversions) {
    chordInversions = inversions;
    update();
}

void HorizontalFretboardWidget::paintEvent(QPaintEvent*) {
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
    // Draw strings (horizontal lines)
    for (int s = 0; s < numStrings; ++s) {
        int y = marginY + s * stringSpacing;
        p.drawLine(marginX, y, w - marginX, y);
    }
    // For degree coloring, parse chord notes from shape.name if possible (e.g. "Cmaj7 (C, E, G, B)")
    for (const auto& shape : chordInversions) {
        std::vector<QString> chordNotes;
        int l = shape.name.indexOf('(');
        int r = shape.name.indexOf(')');
        if (l != -1 && r != -1 && r > l) {
            QString notesStr = shape.name.mid(l+1, r-l-1);
            QStringList noteList = notesStr.split(',', Qt::SkipEmptyParts);
            for (QString n : noteList) chordNotes.push_back(n.trimmed());
        }
        for (int s = 0; s < numStrings; ++s) {
            int fret = shape.frets.size() > s ? shape.frets[s] : -1;
            if (fret > 0 && fret <= numFrets) {
                int x = marginX + fret * fretSpacing - fretSpacing / 2;
                int y = marginY + s * stringSpacing;
                // Determine note name for this position
                static const int stringMidi[6] = {40, 45, 50, 55, 59, 64};
                int midi = stringMidi[s] + fret;
                int noteClass = midi % 12;
                static const char* noteNames[12] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
                QString noteName = noteNames[noteClass];
                int degree = -1;
                QColor color = Qt::gray;
                if (!chordNotes.empty()) {
                    auto dc = getNoteDegreeAndColor(noteName, chordNotes);
                    degree = dc.first;
                    color = dc.second;
                }
                p.setBrush(color);
                p.setPen(Qt::black);
                p.drawEllipse(QPoint(x, y), 8, 8);
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
            // Optionally, mark muted/open strings above nut
            if (fret == 0) {
                int x = marginX - 18;
                int y = marginY + s * stringSpacing + 5;
                p.setPen(Qt::black);
                p.drawText(x, y, "o");
            } else if (fret == -1) {
                int x = marginX - 18;
                int y = marginY + s * stringSpacing + 5;
                p.setPen(Qt::black);
                p.drawText(x, y, "x");
            }
        }
    }
    // Draw chord names for each inversion (above fretboard)
    int labelY = marginY - 30;
    int labelX = marginX;
    for (const auto& shape : chordInversions) {
        if (!shape.name.isEmpty()) {
            p.drawText(labelX, labelY, shape.name);
            labelX += 80; // space between inversion names
        }
    }
}
