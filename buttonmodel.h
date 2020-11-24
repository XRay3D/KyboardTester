#pragma once

#include <QAbstractTableModel>
#include <QVector>

struct Button {
    QString name;
    QPointF position;
    double resistance = 0.0;
};

class ButtonModel : public QAbstractTableModel {
    Q_OBJECT

    QVector<Button> m_data;

public:
    explicit ButtonModel(QObject* parent = nullptr);
    void add() { }
    void sub() { }
    void save() { }
    void open() { }

signals:

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
};
