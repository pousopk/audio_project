#pragma once

#include <QWidget>

/**
 * @brief Widget for displaying compressor gain reduction in dB.
 */
class GainReductionMeter : public QWidget {
    Q_OBJECT
public:
    /** @brief Construct a GainReductionMeter. */
    explicit GainReductionMeter(QWidget* parent = nullptr);
    /** @brief Set the gain reduction value in dB (should be <= 0). */
    void setGainReduction(float db);

protected:
    /** @brief Paint the gain reduction meter. */
    void paintEvent(QPaintEvent* event) override;

private:
    float gain_reduction_db_ = 0.0f;
};