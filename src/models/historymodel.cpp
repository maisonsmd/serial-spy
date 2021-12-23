#include "historymodel.h"
#include <QDebug>
#include <QTextStream>

char * char2hex (char c) {
    static char buffer[3];
    const char * const hex = "0123456789ABCDEF";
    memset(buffer, 0, sizeof(buffer));
    buffer[0] = hex[c >> 4];
    buffer[1] = hex[c & 0x0F];
    return buffer;
}

HistoryModel::HistoryModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

QVariant HistoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case TimestampRole:
            return "Timestamp";
        case DirectionRole:
            return "Dir";
        case HexRole:
            return "Hex";
        case StringRole:
            return "String";
        default:
            break;
        }
    }

    if (role == Qt::DisplayRole && orientation == Qt::Vertical) {
        if (section >= 0 && section < rowCount())
            return m_items.at(section).index;
        else
            return 0;
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

int HistoryModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_items.count();
}

int HistoryModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return ColumnRoles::NumColumns;
}

QVariant HistoryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() < 0 || index.row() >= rowCount())
        return QVariant();

    if (role == Qt::DisplayRole) {
        const auto &item = m_items.at(index.row());

        switch (index.column()) {
        case TimestampRole:
            return item.datetime.toString("HH:mm:ss.zzz");
        case DirectionRole:
            return toString(item.direction);
        case HexRole:
            return formattedHexString(item.data);
        case StringRole:
            return formattedString(item.data);
        default:
            break;
        }
    }
    return QVariant();
}

bool HistoryModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (row < 0 || row >= rowCount())
        return false;

    beginRemoveRows(parent, row, row + count - 1);
    for (int i = 0; i < count; ++i) {
        qDebug() << "removing";
        m_items.removeFirst();
    }
    endRemoveRows();

    return true;
}

void HistoryModel::clear()
{
    m_totalLines = 0;
    m_items.clear();
}

void HistoryModel::setHistoryCapacity(int _cap)
{
    if (_cap == m_historyCapacity)
        return;
    if (_cap < m_historyCapacity) {
        removeRows(0, m_historyCapacity - _cap);
    }
    m_historyCapacity = _cap;
}

void HistoryModel::addItem(DataDirection _dir, const QByteArray &_data)
{
    if (rowCount() == historyCapacity() - 1)
        removeRow(0);

    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_items.append(LogData {m_totalLines, _dir, QDateTime::currentDateTime(), _data});
    endInsertRows();
    ++m_totalLines;
}

void HistoryModel::appendData(DataDirection _dir, const QByteArray &_data)
{
    if (m_endedAtNewline || m_lastDataDirection != _dir) {
        m_endedAtNewline = false;
        addItem(_dir, _data);
    } else {
        Q_ASSERT_X(rowCount() != 0, "appendData", "rowCount cannot be zero here!");
        auto & lastItem = m_items[rowCount() - 1];
        lastItem.data += _data;
    }

    if (_data.endsWith("\n")) {
        m_endedAtNewline = true;
    }

    emit dataChanged(index(rowCount() - 1, HexRole), index(rowCount() - 1, StringRole));
    m_lastDataDirection = _dir;
}

bool HistoryModel::isTextConvertible(const QByteArray &_data)
{
    return true;
}

QString HistoryModel::formattedHexString(const QByteArray &_data) const
{
    auto ret = _data.toHex(' ').toUpper();
    const auto retLen = ret.length();
    const auto newLineCount = newlineAfterCount();

    constexpr auto DISPLAY_CHARACTER_EACH_BYTE = 3;

    if (newLineAfterCountEnabled() &&  newLineCount > 0 && retLen > DISPLAY_CHARACTER_EACH_BYTE) {
        for (int i = newLineCount; i * DISPLAY_CHARACTER_EACH_BYTE < retLen; i += newLineCount) {
            ret.replace(i * DISPLAY_CHARACTER_EACH_BYTE - 1, 1, "\n");
            Q_ASSERT_X(retLen == ret.length(), "replace", "length mismatched");
        }
    }

    return ret;
}

QString HistoryModel::formattedString(const QByteArray &_data) const
{
    QString ret;
    QTextStream stream(&ret);

    for (int i = 0; i < _data.count(); ++i) {
        const auto c = _data.at(i);
        // is c printable?
        if (c >= 32) {
            stream << c;
        } else {
#ifdef DISPLAY_UNPRINTABLE_CHAR_IN_STRING
            ret << '[' << char2hex(c) << ']';
#else
            stream << '.';
#endif
        }
    }

    const auto retLen = ret.length();
    const auto newLineCount = newlineAfterCount();

    if (newLineAfterCountEnabled() &&  newLineCount > 0 && retLen > 1) {
        for (int i = newLineCount; i < retLen; i += newLineCount) {
            ret.replace(i - 1, 1, "\n");
            Q_ASSERT_X(retLen == ret.length(), "replace", "length mismatched");
        }
    }

    return ret;
}

const char * HistoryModel::toString(const DataDirection _dir)
{
    switch (_dir) {
    case DataDirection::A_TO_B:
        return "A->B";
    case DataDirection::B_TO_A:
        return "B->A";
    case DataDirection::A_TO_PC:
        return "A-> ";
    case DataDirection::B_TO_PC:
        return "B-> ";
    case DataDirection::PC_TO_A:
        return " ->A";
    case DataDirection::PC_TO_B:
        return " ->B";
    default:
        break;
    }
    return "Invalid";
}

int HistoryModel::newlineAfterCount() const
{
    return m_newlineAfterCount;
}

void HistoryModel::setNewlineAfterCount(int newNewlineAfterCount)
{
    if (m_newlineAfterCount == newNewlineAfterCount)
        return;
    m_newlineAfterCount = newNewlineAfterCount;
}

bool HistoryModel::newLineAfterCountEnabled() const
{
    return m_newLineAfterCountEnabled;
}

void HistoryModel::setNewlineAfterCountEnabled(bool newNewLineAfterCountEnabled)
{
    if (m_newLineAfterCountEnabled == newNewLineAfterCountEnabled)
        return;
    m_newLineAfterCountEnabled = newNewLineAfterCountEnabled;
}

int HistoryModel::historyCapacity() const
{
    return m_historyCapacity;
}
