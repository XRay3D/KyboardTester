#pragma once

#include <QAbstractTableModel>
#include <QVector>

struct Button {
    QString name;
    QPointF position;
    int resistance = 0;
};

class ButtonModel : public QAbstractTableModel {
    Q_OBJECT

    QVector<Button> m_data;
    QString m_fileName;

public:
    explicit ButtonModel(QObject* parent = nullptr);
    void addButton(QStringView name, QPointF pos);
    void removeButtons(const QModelIndexList& mIdxList);
    void save(const QString& name = {});
    void open(const QString& name = {});

signals:

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QString fileName() const;
    void setFileName(const QString& fileName);
};
