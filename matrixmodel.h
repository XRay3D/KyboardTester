#ifndef MYTABLEMODEL_H
#define MYTABLEMODEL_H

#include <QAbstractTableModel>

#include <hwinterface/amk_tester.h>

class MatrixModel : public QAbstractTableModel {
    Q_OBJECT

    Pins m_data;

public:
    explicit MatrixModel(QObject* parent = nullptr);
    enum {
        ColumnCount = 11,
        RowCount = 11,
    };
    void setDataA(const Pins& value);

    // QAbstractItemModel interface
public:
    int columnCount(const QModelIndex& parent) const override;
    int rowCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
};

#endif // MYTABLEMODEL_H
