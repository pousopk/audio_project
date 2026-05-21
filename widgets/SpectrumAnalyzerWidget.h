#pragma once

#include <QWidget>
#include <vector>

class SpectrumAnalyzerWidget : public QWidget {
    Q_OBJECT
public:
    explicit SpectrumAnalyzerWidget(QWidget* parent = nullptr);
    void setData(const std::vector<float>& magnitudes);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    std::vector<float> spectrum_data_;
};