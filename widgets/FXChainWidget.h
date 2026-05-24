#pragma once

#include <QWidget>
#include <QStringList>

class QListWidget;

/**
 * @brief Widget for managing the order of audio effects in the FX chain.
 */
class FXChainWidget : public QWidget {
    Q_OBJECT
public:
    explicit FXChainWidget(QWidget* parent = nullptr);

    void setOrder(const QStringList& newOrder);
    void addItem(const QString& item);
    void removeItem(int index);

    QString currentItemText() const;
    int currentRow() const;

    QStringList currentOrder() const; // Add method to retrieve the current order

signals:
    void orderChanged(const QStringList& newOrder);

private:
    QListWidget* effectsList_ = nullptr;
};