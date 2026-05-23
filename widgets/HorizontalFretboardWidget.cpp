#include "HorizontalFretboardWidget.h"
#include <QPainter>
#include <QFont>
#include <algorithm>
#include "../utils/NoteColorUtils.h"
#include <QMap> // For QMap in getIntervalAndColor

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
        const QString shapeName = QString::fromStdString(shape.name);
        const std::string shapeNameStd = shape.name;
        // Determine the root note from the shape.name (e.g., "C#m7" -> "C#")
        std::string rootNote = shapeNameStd.substr(0, 1);
        if (shapeNameStd.length() > 1 && (shapeNameStd[1] == '#' || shapeNameStd[1] == 'b')) {
            rootNote = shapeNameStd.substr(0, 2);
        }

        for (int s = 0; s < numStrings; ++s) {
            const std::size_t stringIndex = static_cast<std::size_t>(s);
            int fret = (stringIndex < shape.frets.size()) ? shape.frets[stringIndex] : -1;
            if (fret > 0 && fret <= numFrets) {
                int x = marginX + fret * fretSpacing - fretSpacing / 2;
                int y = marginY + s * stringSpacing;
                // Determine note name for this position
                static const int stringMidi[6] = {40, 45, 50, 55, 59, 64};
                int midi = stringMidi[s] + fret;
                int noteClass = midi % 12;
                static const char* noteNames[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
                std::string noteName = noteNames[noteClass];

                // Get interval and color using the new function
                auto intervalAndColor = getIntervalAndColor(noteName, rootNote);
                QString intervalText = QString::fromStdString(intervalAndColor.first);
                const NoteColor& colorData = intervalAndColor.second;
                QColor color(colorData.r, colorData.g, colorData.b);

                p.setBrush(color);
                p.setPen(Qt::black);
                p.drawEllipse(QPoint(x, y), 8, 8);
                p.setPen(Qt::white);
                QFont font = p.font();
                font.setPointSize(7);
                p.setFont(font);
                p.drawText(QRect(x - 8, y - 8, 16, 16), Qt::AlignCenter, intervalText);
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
        if (!shape.name.empty()) {
            p.drawText(labelX, labelY, QString::fromStdString(shape.name));
            labelX += 80; // space between inversion names
        }
    }
}
