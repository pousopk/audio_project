#include "GuitarFretboardWidget.h"
#include <QPainter>
#include <QFont>

GuitarFretboardWidget::GuitarFretboardWidget(QWidget* parent)
    : QWidget(parent) {
    setMinimumSize(200, 300);
}

void GuitarFretboardWidget::setChord(const GuitarChordShape& shape) {
    currentShape = shape;
    update();
}

void GuitarFretboardWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    int w = width();
    int h = height();
    int margin = 30;
    int fretSpacing = (h - 2 * margin) / numFrets;
    int stringSpacing = (w - 2 * margin) / (numStrings - 1);

    // Draw frets
    for (int f = 0; f <= numFrets; ++f) {
        int y = margin + f * fretSpacing;
        p.drawLine(margin, y, w - margin, y);
    }
    // Draw strings (flipped: string 0 is rightmost, string 5 is leftmost)
    for (int s = 0; s < numStrings; ++s) {
        int x = margin + (numStrings - 1 - s) * stringSpacing;
        p.drawLine(x, margin, x, h - margin);
    }
    // Draw nut (if open chord)
    if (!currentShape.frets.empty() && *std::min_element(currentShape.frets.begin(), currentShape.frets.end()) <= 1) {
        p.setPen(QPen(Qt::black, 4));
        p.drawLine(margin, margin, w - margin, margin);
        p.setPen(QPen(Qt::black, 1));
    }
    // Draw finger positions (flipped)
    for (int s = 0; s < numStrings; ++s) {
        const std::size_t stringIndex = static_cast<std::size_t>(s);
        int fret = stringIndex < currentShape.frets.size() ? currentShape.frets[stringIndex] : -1;
        int x = margin + (numStrings - 1 - s) * stringSpacing;
        if (fret == -1) {
            // Muted string
            p.drawText(x - 7, margin - 10, "x");
        } else if (fret == 0) {
            // Open string
            p.drawText(x - 7, margin - 10, "o");
        } else if (fret > 0 && fret <= numFrets) {
            int y = margin + fret * fretSpacing - fretSpacing / 2;
            p.setBrush(Qt::black);
            p.drawEllipse(QPoint(x, y), 8, 8);
        }
    }
    // Draw chord name
    if (!currentShape.name.isEmpty()) {
        QFont font = p.font();
        font.setPointSize(14);
        font.setBold(true);
        p.setFont(font);
        p.drawText(rect(), Qt::AlignTop | Qt::AlignHCenter, currentShape.name);
    }
}
