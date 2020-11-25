#include "buttonmodel.h"

#include <QColor>
#include <QDataStream>
#include <QDebug>
#include <QFile>

QDataStream& operator>>(QDataStream& stream, Button& button)
{
    stream >> button.name;
    stream >> button.position;
    stream >> button.resistance;
    return stream;
}
QDataStream& operator<<(QDataStream& stream, const Button& button)
{
    stream << button.name;
    stream << button.position;
    stream << button.resistance;
    return stream;
}

QString ButtonModel::fileName() const
{
    return m_fileName;
}

void ButtonModel::setFileName(const QString& fileName)
{
    m_fileName = fileName;
}

ButtonModel::ButtonModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

void ButtonModel::addButton(QStringView name, QPointF pos)
{
    beginInsertRows({}, m_data.size(), m_data.size());
    m_data.append({ name.toString(), pos, 0 });
    endInsertRows();
}

void ButtonModel::removeButtons(const QModelIndexList& mIdxList)
{
    beginRemoveRows({}, m_data.size() - mIdxList.size() / 3, m_data.size() - 1);
    for (auto& index : mIdxList) {
        if (!index.column())
            m_data.remove(index.row());
    }
    endRemoveRows();
}

void ButtonModel::save(const QString& name)
{
    if (!name.isEmpty())
        m_fileName = name;
    QFile file(m_fileName);
    if (file.open(QFile::WriteOnly)) {
        QDataStream out(&file);
        out << m_data;
        m_isNew = false;
    }
}

void ButtonModel::open(const QString& name)
{
    m_fileName = name;
    QFile file(m_fileName);
    if (file.open(QFile::ReadOnly)) {
        if (const int size = m_data.size(); size) {
            beginRemoveRows({}, 0, size - 1);
            endRemoveRows();
        }
        QDataStream in(&file);
        in >> m_data;

        beginInsertRows({}, 0, m_data.size() - 1);
        endInsertRows();
    }
}

void ButtonModel::newOrClose()
{
    m_isNew = true;
    m_fileName.clear();
    beginRemoveRows({}, 0, m_data.size() - 1);
    m_data.clear();
    endRemoveRows();
}

void ButtonModel::setResistance(int index, int resistance)
{
    m_data[index].resistance = resistance;
    dataChanged(createIndex(index, 2), createIndex(index, 2), {});
}

int ButtonModel::rowCount(const QModelIndex& /*parent*/) const { return m_data.size(); }

int ButtonModel::columnCount(const QModelIndex& /*parent*/) const { return 3; }

QVariant ButtonModel::data(const QModelIndex& index, int role) const
{
    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case 0:
            return m_data[index.row()].name;
        case 1: {
            QPointF pos(m_data[index.row()].position);
            return QString("XY(  %1  ,  %2  )").arg(pos.x()).arg(pos.y());
        }
        case 2:
            return m_data[index.row()].resistance;
        }
    case Qt::EditRole:
        switch (index.column()) {
        case 0:
            return m_data[index.row()].name;
        case 1:
            return m_data[index.row()].position;
        case 2:
            return m_data[index.row()].resistance;
        }
    case Qt::TextColorRole:
        if (index.column() == 2) {
            return (m_data[index.row()].resistance > 100 || m_data[index.row()].resistance < 1)
                ? QColor(Qt::red)
                : QColor(Qt::darkGreen);
        }
        return {};

    case Qt::TextAlignmentRole:
        return Qt::AlignCenter;
    }
    return {};
}

bool ButtonModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    switch (role) {
    case Qt::EditRole:
        switch (index.column()) {
        case 0:
            m_data[index.row()].name = value.toString();
            return true;
        case 1:
            m_data[index.row()].position = value.toPointF();
            return true;
        case 2:
            m_data[index.row()].resistance = value.toDouble();
            return true;
        }
    }
    return false;
}

QVariant ButtonModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return "Имя";
        case 1:
            return "Позиция";
        case 2:
            return "Сопротивление";
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

Qt::ItemFlags ButtonModel::flags(const QModelIndex& /*index*/) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
}
