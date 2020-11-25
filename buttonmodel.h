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
    bool m_isNew = true;

public:
    explicit ButtonModel(QObject* parent = nullptr);
    void addButton(QStringView name, QPointF pos);
    void removeButtons(const QModelIndexList& mIdxList);
    void save(const QString& name = {});
    void open(const QString& name);
    void newOrClose();
    bool isNew() { return m_isNew; }
    const QVector<Button>& buttons() const { return m_data; }
    void setResistance(int index, int resistance);

    // QAbstractItemModel interface
    int rowCount(const QModelIndex& = {}) const override;
    int columnCount(const QModelIndex& = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QString fileName() const;
    void setFileName(const QString& fileName);
};
