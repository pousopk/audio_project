#pragma once

#include <QWidget>
#include <QStringList>

class QListWidget;

class FXChainWidget : public QWidget {
    Q_OBJECT
public:
    explicit FXChainWidget(QWidget* parent = nullptr);
    void setOrder(const QStringList& newOrder);
    void addItem(const QString& item);
    void removeItem(int index);
    QString currentItemText() const;
    int currentRow() const;

signals:
    void orderChanged(const QStringList& newOrder);

private:
    QListWidget* effectsList_;
};