
#pragma once
#include <QWidget>
#include <QPushButton>
#include <vector>
#include "ChordProgressionManager.h"

class QListWidget;
class QPushButton;
class QLineEdit;
class QSlider;
class QLabel;
class QComboBox;

/**
 * @brief Widget for managing and editing chord progressions.
 */
class ChordProgressionWidget : public QWidget {
    Q_OBJECT
public:
    /** @brief Construct a ChordProgressionWidget. */
    explicit ChordProgressionWidget(QWidget* parent = nullptr);
    /** @brief Get the progression manager instance. */
    ChordProgressionManager* progressionManager();

    /** @brief Get the selected scale root. */
    QString scaleRoot() const;
    /** @brief Get the selected scale type. */
    QString scaleType() const;

signals:
    void progressionChanged(const std::vector<ChordChange>& progression);
    void playRequested(const std::vector<ChordChange>& progression);
    void stopRequested();
    void chordVolumeChanged(double volume);
    void scaleChanged(const QString& root, const QString& name);

private slots:
    void addChord();
    void removeSelectedChord();
    void saveProgression();
    void loadProgression();
    void setStandardProgression();
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
    QComboBox* scaleRootCombo_;
    QComboBox* scaleTypeCombo_;
    QSlider* chordVolumeSlider_;
    QLabel* chordVolumeValue_;
    void refreshSavedCombo();
    void refreshStandardCombo();
};
