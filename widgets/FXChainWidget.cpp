#include "FXChainWidget.h"
#include <QListWidget>
#include <QVBoxLayout>
#include <QLabel>

FXChainWidget::FXChainWidget(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 5, 0, 5);
    QLabel* title = new QLabel("FX Chain (Drag to Reorder)", this);
    layout->addWidget(title);

    effectsList_ = new QListWidget(this);
    effectsList_->setDragDropMode(QAbstractItemView::InternalMove);
    effectsList_->setDefaultDropAction(Qt::MoveAction);
    effectsList_->setSelectionMode(QAbstractItemView::SingleSelection);
    effectsList_->setMaximumHeight(80);

    layout->addWidget(effectsList_);
    setLayout(layout);

    connect(effectsList_->model(), &QAbstractItemModel::rowsMoved, this, [this]() {
        QStringList newOrder;
        for (int i = 0; i < effectsList_->count(); ++i) {
            newOrder.append(effectsList_->item(i)->text());
        }
        emit orderChanged(newOrder);
    });
}

void FXChainWidget::setOrder(const QStringList& newOrder) {
    effectsList_->clear();
    effectsList_->addItems(newOrder);
    // The rowsMoved signal won't be emitted, so we must emit our own signal
    emit orderChanged(newOrder);
}

void FXChainWidget::addItem(const QString& item) {
    effectsList_->addItem(item);
    QStringList newOrder;
    for (int i = 0; i < effectsList_->count(); ++i) {
        newOrder.append(effectsList_->item(i)->text());
    }
    emit orderChanged(newOrder);
}

void FXChainWidget::removeItem(int index) {
    delete effectsList_->takeItem(index);
    QStringList newOrder;
    for (int i = 0; i < effectsList_->count(); ++i) {
        newOrder.append(effectsList_->item(i)->text());
    }
    emit orderChanged(newOrder);
}

QString FXChainWidget::currentItemText() const {
    if (effectsList_->currentItem()) return effectsList_->currentItem()->text();
    return QString();
}

int FXChainWidget::currentRow() const {
    return effectsList_->currentRow();
}

QStringList FXChainWidget::currentOrder() const {
    QStringList order;
    for (int i = 0; i < effectsList_->count(); ++i) {
        order.append(effectsList_->item(i)->text());
    }
    return order;
}