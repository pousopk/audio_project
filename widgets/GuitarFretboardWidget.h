
#pragma once
#include <QWidget>
#include <vector>
#include <QString>
#include "../include/GuitarChordShape.h"

class GuitarFretboardWidget : public QWidget {
    Q_OBJECT
public:
    explicit GuitarFretboardWidget(QWidget* parent = nullptr);
    void setChord(const GuitarChordShape& shape);
protected:
    void paintEvent(QPaintEvent* event) override;
private:
    GuitarChordShape currentShape;
    int numFrets = 5;
    int numStrings = 6;
};
