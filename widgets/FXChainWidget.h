#pragma once

#include <QWidget>
#include <QStringList>

class QListWidget;

/**
 * @brief Widget for managing the order of effects in the FX chain.
 */
class FXChainWidget : public QWidget {
    Q_OBJECT
public:
    /** @brief Construct an FXChainWidget. */
    explicit FXChainWidget(QWidget* parent = nullptr);
    /** @brief Set the order of effects. */
    void setOrder(const QStringList& newOrder);
    /** @brief Add an effect item. */
    void addItem(const QString& item);
    /** @brief Remove an effect at the given index. */
    void removeItem(int index);
    /** @brief Get the currently selected item text. */
    QString currentItemText() const;
    /** @brief Get the current row index. */
    int currentRow() const;

signals:
    void orderChanged(const QStringList& newOrder);

private:
    QListWidget* effectsList_;
};