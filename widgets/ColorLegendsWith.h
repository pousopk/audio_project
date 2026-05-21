#pragma once
#include <QWidget>

class ColorLegendsWith : public QWidget {
    Q_OBJECT
public:
    explicit ColorLegendsWith(QWidget* parent = nullptr);
protected:
    void paintEvent(QPaintEvent* event) override;
};