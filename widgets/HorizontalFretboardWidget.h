
#pragma once
#include <QWidget>
#include <vector>
#include <QString>
#include "../include/GuitarChordShape.h"

/**
 * @brief Widget for displaying horizontal guitar fretboard and chord inversions.
 */
class HorizontalFretboardWidget : public QWidget {
    Q_OBJECT
public:
    /** @brief Construct a HorizontalFretboardWidget. */
    explicit HorizontalFretboardWidget(QWidget* parent = nullptr);
    /** @brief Set the chord inversions to display. */
    void setChordInversions(const std::vector<GuitarChordShape>& inversions);
protected:
    /** @brief Paint the fretboard and chord shapes. */
    void paintEvent(QPaintEvent* event) override;
private:
    std::vector<GuitarChordShape> chordInversions;
    int numFrets = 24;
    int numStrings = 6;
};
