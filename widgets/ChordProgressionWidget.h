
#pragma once
#include <QWidget>
#include <QPushButton>
#include "ChordProgressionManager.h"

class QListWidget;
class QPushButton;
class QLineEdit;
class QComboBox;

class ChordProgressionWidget : public QWidget {
    Q_OBJECT
public:
    explicit ChordProgressionWidget(QWidget* parent = nullptr);
    ChordProgressionManager* progressionManager();

signals:
    void progressionChanged(const QVector<ChordChange>& progression);
    void playRequested(const QVector<ChordChange>& progression);
    void stopRequested();

private slots:
    void addChord();
    void removeSelectedChord();
    void saveProgression();
    void loadProgression();
    void setStandardProgression();
    void appendStandardToList();
    void updateProgressionList();
    void playProgression();
    void stopProgression();

private:
    ChordProgressionManager manager_;
    QListWidget* chordList_;
    QComboBox* rootCombo_;
    QComboBox* typeCombo_;
    QLineEdit* barsEdit_;
    QPushButton* addButton_;
    QPushButton* removeButton_;
    QPushButton* saveButton_;
    QPushButton* loadButton_;
    QPushButton* stopButton_;
    QComboBox* savedCombo_;
    QComboBox* standardCombo_;
    QPushButton* appendStandardButton_;
    QPushButton* playButton_;
    QComboBox* strumCombo_;
    void refreshSavedCombo();
    void refreshStandardCombo();
};
