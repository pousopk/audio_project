#pragma once

#include <QWidget>
#include <vector>

/**
 * @brief Widget for displaying a real-time spectrum analyzer.
 */
class SpectrumAnalyzerWidget : public QWidget {
    Q_OBJECT
public:
    /** @brief Construct a SpectrumAnalyzerWidget. */
    explicit SpectrumAnalyzerWidget(QWidget* parent = nullptr);
    /** @brief Set the spectrum data to display. */
    void setData(const std::vector<float>& magnitudes);

protected:
    /** @brief Paint the spectrum analyzer. */
    void paintEvent(QPaintEvent* event) override;

private:
    std::vector<float> spectrum_data_;
};