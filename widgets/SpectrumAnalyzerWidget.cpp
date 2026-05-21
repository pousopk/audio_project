#include "SpectrumAnalyzerWidget.h"
#include <QPainter>
#include <QPainterPath>
#include <cmath>

SpectrumAnalyzerWidget::SpectrumAnalyzerWidget(QWidget* parent) : QWidget(parent) {
    setMinimumHeight(120);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

void SpectrumAnalyzerWidget::setData(const std::vector<float>& magnitudes) {
    spectrum_data_ = magnitudes;
    update();
}

void SpectrumAnalyzerWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter p(this);

    p.fillRect(rect(), QColor(30, 30, 30));

    if (spectrum_data_.empty()) {
        return;
    }

    p.setPen(QColor(100, 200, 255));

    float max_mag = -100.0f;
    for (float val : spectrum_data_) {
        if (val > max_mag) max_mag = val;
    }

    QPainterPath path;
    path.moveTo(0, height());

    for (size_t i = 0; i < spectrum_data_.size(); ++i) {
        // Logarithmic scale for frequency (x-axis)
        float x_log = log10(static_cast<float>(i + 1)) / log10(static_cast<float>(spectrum_data_.size()));
        float x = x_log * width();

        // Linear scale for magnitude (y-axis), from -60dB to 0dB
        float y_lin = (spectrum_data_[i] + 60.0f) / 60.0f;
        float y = height() * (1.0f - std::clamp(y_lin, 0.0f, 1.0f));
        path.lineTo(x, y);
    }
    path.lineTo(width(), height());
    path.closeSubpath();

    p.fillPath(path, QColor(100, 200, 255, 100));
    p.drawPath(path);
}