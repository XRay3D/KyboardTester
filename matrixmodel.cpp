#include "matrixmodel.h"
#include <QColor>
#include <QDebug>
#include <QMutex>

MatrixModel::MatrixModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

int MatrixModel::isOk()
{
    QVector<int> res;
    res.reserve(11 * 11);
    for (int r = 0; r < 11; ++r) {
        for (int c = 0; c < 11; ++c) {
            if (m_data[r][c] > 0)
                res << m_data[r][c];
        }
    }
    std::ranges::sort(res);
    //    qDebug() << d;
    return res.size() == 2
        ? res.first()
        : (res.size() > 2
                ? -1
                : 0);
}

void MatrixModel::setDataA(const Pins& value)
{
    m_data = value;
    dataChanged(createIndex(0, 0), createIndex(10, 10), { Qt::DisplayRole });
}

int MatrixModel::rowCount(const QModelIndex& /*parent*/) const
{
    return RowCount;
}

int MatrixModel::columnCount(const QModelIndex& /*parent*/) const
{
    return ColumnCount;
}

QVariant MatrixModel::data(const QModelIndex& index, int role) const
{
    switch (role) {
    case Qt::DisplayRole:
        if (m_data[index.row()][index.column()] > -1)
            return m_data[index.row()][index.column()];
        return QVariant();
    case Qt::TextAlignmentRole:
        return Qt::AlignCenter;
    case Qt::BackgroundColorRole:
        if (m_data[index.row()][index.column()] > -1)
            return QColor(255, 100, 100);
        return QVariant();
        //        else if (index.row() < 6 && index.column() < 6)
        //            return QColor(220, 255, 220);
        //        else if (index.row() > 5 && index.column() > 5)
        //            return QColor(255, 220, 220);
        //        else
        //            return QColor(255, 255, 220);
    default:
        return QVariant();
    }
}

QVariant MatrixModel::headerData(int section, Qt::Orientation /*orientation*/, int role) const
{
    //const QStringList header({ "K1", "K2", "K3", "K4", "-U", "+U", "+I", "-I", "mV", "V", "-V" });
    if (role == Qt::DisplayRole)
        return QString::number(section);
    if (role == Qt::TextAlignmentRole)
        return Qt::AlignCenter;
    return QVariant();
}

Qt::ItemFlags MatrixModel::flags(const QModelIndex& /*index*/) const
{
    return Qt::ItemIsEnabled;
}
