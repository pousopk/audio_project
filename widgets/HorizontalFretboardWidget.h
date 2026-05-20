
#pragma once
#include <QWidget>
#include <vector>
#include <QString>
#include "../include/GuitarChordShape.h"

class HorizontalFretboardWidget : public QWidget {
    Q_OBJECT
public:
    explicit HorizontalFretboardWidget(QWidget* parent = nullptr);
    void setChordInversions(const std::vector<GuitarChordShape>& inversions);
protected:
    void paintEvent(QPaintEvent* event) override;
private:
    std::vector<GuitarChordShape> chordInversions;
    int numFrets = 24;
    int numStrings = 6;
};
