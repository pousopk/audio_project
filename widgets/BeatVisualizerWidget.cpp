#include "BeatVisualizerWidget.h"
#include <QPainter>
#include <QFont>

BeatVisualizerWidget::BeatVisualizerWidget(QWidget *parent)
    : QWidget(parent) {
    setMinimumHeight(100);
    setMinimumWidth(300);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void BeatVisualizerWidget::setBeatsPerBar(int beats) {
    beatsPerBar = beats;
    update();
}

void BeatVisualizerWidget::setCurrentBeat(int beatIdx) {
    currentBeat = beatIdx;
    update();
}

void BeatVisualizerWidget::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    int w = width();
    int h = height();
    painter.fillRect(rect(), QColor(240,240,240));
    int spacing = w / (beatsPerBar + 1);
    QFont font = painter.font();
    font.setPointSize(h / 2);
    painter.setFont(font);
    for (int i = 0; i < beatsPerBar; ++i) {
        QRect rect((i + 1) * spacing - spacing / 2, h / 4, spacing, h / 2);
        QColor color = normalColor;
        if (i == currentBeat) {
            if (i == 0) {
                color = highlightColor; // Red for strong beat
            } else {
                color = activeColor; // Blue for weak beats
            }
        }
        painter.setPen(Qt::NoPen);
        painter.setBrush(color);
        painter.drawEllipse(rect.center(), spacing / 2, h / 3);
        painter.setPen(Qt::white);
        painter.drawText(rect, Qt::AlignCenter, QString::number(i + 1));
    }
}
