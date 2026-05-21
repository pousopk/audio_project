#pragma once
#include <QWidget>

/**
 * @brief Widget for displaying color legends for notes or intervals.
 */
class ColorLegendsWith : public QWidget {
    Q_OBJECT
public:
    /** @brief Construct a ColorLegendsWith widget. */
    explicit ColorLegendsWith(QWidget* parent = nullptr);
protected:
    /** @brief Paint the color legend. */
    void paintEvent(QPaintEvent* event) override;
};