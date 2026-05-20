#pragma once
#include <QWidget>
#include <QColor>

class BeatVisualizerWidget : public QWidget {
    Q_OBJECT
public:
    BeatVisualizerWidget(QWidget *parent = nullptr);
    void setBeatsPerBar(int beats);
    void setCurrentBeat(int beatIdx);
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    int beatsPerBar = 4;
    int currentBeat = 0;
    QColor normalColor = QColor(0, 0, 0); // Black for all
    QColor highlightColor = QColor(200, 0, 0); // Red for strong beat
    QColor activeColor = QColor(60, 120, 220); // Blue for weak beats
};
