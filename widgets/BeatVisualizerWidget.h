#pragma once
#include <QWidget>
#include <QColor>

/**
 * @brief Widget for visualizing beats in a measure.
 */
class BeatVisualizerWidget : public QWidget {
    Q_OBJECT
public:
    /** @brief Construct a BeatVisualizerWidget. */
    BeatVisualizerWidget(QWidget *parent = nullptr);
    /** @brief Set the number of beats per bar. */
    void setBeatsPerBar(int beats);
    /** @brief Set the current beat index. */
    void setCurrentBeat(int beatIdx);
protected:
    /** @brief Paint the beat visualization. */
    void paintEvent(QPaintEvent *event) override;
private:
    int beatsPerBar = 4;
    int currentBeat = 0;
    QColor normalColor = QColor(0, 0, 0); // Black for all
    QColor highlightColor = QColor(200, 0, 0); // Red for strong beat
    QColor activeColor = QColor(60, 120, 220); // Blue for weak beats
};
