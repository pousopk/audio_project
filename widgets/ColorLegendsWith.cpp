#include "ColorLegendsWith.h"
#include <QPainter>
#include <vector>
#include <QString>
#include <QColor>

struct LegendItem {
    QColor color;
    QString label;
    bool isOutline = false;
};

const std::vector<LegendItem> legendItems = {
    {QColor(255, 70, 70), "Root (1)"},
    {QColor(150, 150, 255), "Major 3rd"},
    {QColor(100, 100, 255), "Minor 3rd"},
    {QColor(70, 180, 70), "Perfect 5th"},
    {QColor(200, 100, 200), "Minor 7th"},
    {Qt::lightGray, "Other Scale Tones"},
    {QColor(255, 255, 0, 200), "Target/Chord Tone", true}
};

ColorLegendsWith::ColorLegendsWith(QWidget* parent) : QWidget(parent) {
    setFixedHeight(40);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void ColorLegendsWith::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int x = 15;
    int y = height() / 2;
    int circleRadius = 8;
    int spacing = 140;

    for (const auto& item : legendItems) {
        if (item.isOutline) {
            p.setPen(QPen(item.color, 2));
            p.setBrush(Qt::NoBrush);
            p.drawEllipse(QPoint(x, y), circleRadius, circleRadius);
        } else {
            p.setBrush(item.color);
            p.setPen(Qt::black);
            p.drawEllipse(QPoint(x, y), circleRadius, circleRadius);
        }

        p.setPen(Qt::black);
        p.drawText(x + circleRadius + 5, y + 5, item.label);

        x += spacing;
    }
}