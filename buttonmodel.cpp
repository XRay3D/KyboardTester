#include "buttonmodel.h"

ButtonModel::ButtonModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

int ButtonModel::rowCount(const QModelIndex& parent) const
{
    return m_data.size();
}

int ButtonModel::columnCount(const QModelIndex& parent) const
{
    return 3;
}

QVariant ButtonModel::data(const QModelIndex& index, int role) const
{
    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case 0:
            return m_data[index.row()].name;
        case 1:
            return m_data[index.row()].position;
        case 2:
            return m_data[index.row()].resistance;
        }
    }
    return {};
}
