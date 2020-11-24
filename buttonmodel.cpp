#include "buttonmodel.h"

#include <QDataStream>
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
    }
}

void ButtonModel::open(const QString& name)
{
    if (!name.isEmpty())
        m_fileName = name;
    QFile file(m_fileName);
    if (file.open(QFile::ReadOnly)) {
        const int size = m_data.size();
        QDataStream in(&file);
        in >> m_data;
        qDebug("%d", m_data.size());
        //        throw std::exception("Fix it");
        if (size < m_data.size()) {
            beginInsertRows({}, 0, m_data.size());
            endInsertRows();
        } else {
            beginRemoveRows({}, 0, 0);
            endRemoveRows();
        }
    }
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
            return QString("X%1    Y%2").arg(pos.x()).arg(pos.y());
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
    return {};
}

QVariant ButtonModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QAbstractTableModel::headerData(section, orientation, role);
}

Qt::ItemFlags ButtonModel::flags(const QModelIndex& /*index*/) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
}
