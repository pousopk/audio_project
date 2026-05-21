#include "GainReductionMeter.h"
#include <QPainter>
#include <algorithm>

GainReductionMeter::GainReductionMeter(QWidget* parent) : QWidget(parent) {
    setMinimumSize(20, 100);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
}

void GainReductionMeter::setGainReduction(float db) {
    gain_reduction_db_ = std::min(db, 0.0f);
    update();
}

void GainReductionMeter::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter p(this);

    // Background
    p.fillRect(rect(), QColor(50, 50, 50));

    // Draw meter bar
    float max_db = -40.0f; // The bottom of our meter range
    float gr_clamped = std::max(gain_reduction_db_, max_db);

    // Map dB value to a percentage of the widget height
    int meter_height = static_cast<int>((gr_clamped / max_db) * height());
    int meter_y = height() - meter_height;

    QRect meterRect(0, meter_y, width(), meter_height);
    p.fillRect(meterRect, QColor(255, 165, 0)); // Orange color for reduction

    // Border
    p.setPen(QColor(30, 30, 30));
    p.drawRect(rect().adjusted(0, 0, -1, -1));
}