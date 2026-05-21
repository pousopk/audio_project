#pragma once

#include <QWidget>

class GainReductionMeter : public QWidget {
    Q_OBJECT
public:
    explicit GainReductionMeter(QWidget* parent = nullptr);
    void setGainReduction(float db); // Value in dB, should be <= 0

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    float gain_reduction_db_ = 0.0f;
};