#include "historymodel.h"
#include <QDebug>
#include <QTextStream>
#include <QColor>
#include <cmath>
#include <algorithm>

#include "utils/commonconfig.h"

// not thread-safe
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
        case toColumn(TimestampRole):
            return "Timestamp";
        case toColumn(DirectionRole):
            return "Dir";
        case toColumn(HexRole):
            return "Hex";
        case toColumn(StringRole):
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

    return toColumn(ColumnRoles::NumColumns);
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
        case toColumn(TimestampRole):
            return item.createdDatetime.toString(TIME_FORMAT);
        case toColumn(DirectionRole):
            return toString(item.direction);
        case toColumn(HexRole):
            return formattedHexString(item.data);
        case toColumn(StringRole):
            return formattedString(item.data);
        default:
            break;
        }
    }

    if (role == Qt::ForegroundRole) {
        switch (index.column()) {
        case toColumn(TimestampRole):
        case toColumn(HexRole):
        case toColumn(DirectionRole):
            return QColor(Qt::gray);
        default:
            return QColor(Qt::black);
        }
    }

    return QVariant();
}

bool HistoryModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (rowCount() == 0)
        return false;

    if (row < 0 || row >= rowCount())
        return false;

    if (row + count >= rowCount()) {
        count = rowCount() - row;
    }

    beginRemoveRows(parent, row, row + count - 1);
    for (int i = 0; i < count && !m_items.empty(); ++i) {
        m_items.removeFirst();
    }
    endRemoveRows();

    return true;
}

void HistoryModel::clear()
{
    beginResetModel();
    m_totalLines = 0;
    m_items.clear();
    endResetModel();
}

void HistoryModel::setHistoryCapacity(int _cap)
{
    if (_cap == m_historyCapacity)
        return;
    // shrinking
    if (_cap < m_historyCapacity) {
        if (rowCount() > _cap) {
            removeRows(0, _cap - rowCount());
        }
    }
    m_historyCapacity = _cap;
}

void HistoryModel::addItem(DataDirection _dir, const QByteArray &_data)
{
    if (rowCount() + 1 > historyCapacity())
        removeRow(0);

    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_items.append(LogData {m_totalLines, _dir, now(), now(), _data});
    m_totalLines += 1;
    endInsertRows();
}

void HistoryModel::addItems(DataDirection _dir, const QList<QByteArray> &_data)
{
    const auto length = _data.length();
    if (length == 0)
        return;

    if (rowCount() + length > historyCapacity())
        removeRows(0, rowCount() + length - historyCapacity());

    beginInsertRows(QModelIndex(), rowCount(), rowCount() + length - 1);
    for (int i = 0; i < length; ++i) {
        m_items.append(LogData {m_totalLines, _dir, now(), now(), _data[i]});
        m_totalLines += 1;
    }
    endInsertRows();
}

void HistoryModel::appendData(DataDirection _dir, const QByteArray &_data)
{
    if (_data.length() == 0)
        return;

    const auto chunkLength = newlineAfterCount();
    bool needNewline = false;

    if (m_endedAtNewline)
        needNewline = true;
    m_endedAtNewline = false;

    if (rowCount() == 0)
        needNewline = true;

    if (rowCount() > 0) {
        auto &lastItem = m_items[rowCount() - 1];
        if (lastItem.direction != _dir) {
            needNewline = true;
        }

        const auto timeDiff = now().toMSecsSinceEpoch() - lastItem.lastAppendDateTime.toMSecsSinceEpoch();
        if (newlineAfterDurationEnabled() && timeDiff > newlineAfterDuration()) {
            needNewline = true;
        }
    }

    if (needNewline) {
        // add new rows
        addItems(_dir, splitData(_data, newLineAfterCountEnabled(), chunkLength, chunkLength));
    } else {
        auto &lastItem = m_items[rowCount() - 1]; // rowCount() always > 0 here

        // if new data doesn't fit to the previous row,
        // split it, then append the begining to previous row, and the rest to new rows
        if (newLineAfterCountEnabled()) {

            int firstChunkLength = newlineAfterCount();
            bool concatenateFirstChunk = false;

            if (lastItem.data.length() < chunkLength) {
                concatenateFirstChunk = true;
                firstChunkLength = chunkLength - lastItem.data.length();
            }

            auto newItems = splitData(_data, true, chunkLength, firstChunkLength);
            if (concatenateFirstChunk) {
                lastItem.data.append(newItems.first());
                emit dataChanged(index(rowCount() - 1, toColumn(StringRole)), index(rowCount() - 1, toColumn(HexRole)));
                newItems.pop_front();
            }

            addItems(_dir, newItems);
        } else {
            // new data fits to the last row, just append it
            lastItem.data.append(_data);
            lastItem.lastAppendDateTime = now();
            emit dataChanged(index(rowCount() - 1, toColumn(StringRole)), index(rowCount() - 1, toColumn(HexRole)));
        }
    }

    if (_data.endsWith("\n")) {
        m_endedAtNewline = true;
    }
}

QString HistoryModel::formattedHexString(const QByteArray &_data) const
{
    constexpr auto DISPLAY_CHARACTER_EACH_BYTE = 3; // 2 chars for HEX + 1 space
    auto ret = _data.toHex(' ').toUpper(); // TODO: toHex fails if Unicode?

    QList<QByteArray> lines {};

    const auto lineLen = newlineAfterCount() * DISPLAY_CHARACTER_EACH_BYTE;
    lines = splitData(ret, newLineAfterCountEnabled(), lineLen, lineLen);

    const auto maxLen = newlineAfterCount() * DISPLAY_CHARACTER_EACH_BYTE + ((newlineAfterCount() - 1) / 8);
    for (auto &l : lines) {
        l.append(' ');

        auto len = l.length() / DISPLAY_CHARACTER_EACH_BYTE;

        for (int i = len - 1; i >= 8; i = int((i - 1) / 8) * 8) {
            if ((i) % 8 == 0) {
                l.insert((i)* DISPLAY_CHARACTER_EACH_BYTE, " ");
            }
        }

        len = l.length();
        // right padding
        if (len < maxLen) {
            const auto spaces {maxLen - len};
            l.append(QByteArray((spaces), ' '));
        }

    }

    return lines.join("\n");
}

QString HistoryModel::formattedString(const QByteArray &_data) const
{
    QString ret;
    QTextStream stream(&ret);
    int lines = 1;

    for (int i = 0; i < _data.count(); ++i) {
        const auto c = _data.at(i);
        // is c printable?
        if (c >= 32 && c < 127) {
            stream << c;
        } else {
            stream << '.';
        }

        if (newLineAfterCountEnabled() && newlineAfterCount() > 0) {
            if (i + 1 == lines * newlineAfterCount() && i + 1 < _data.length()) {
                stream << "\n";
                lines++;
            }
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

int HistoryModel::newlineAfterDuration() const
{
    return m_newlineAfterDuration;
}

void HistoryModel::setNewlineAfterDuration(int newNewlineAfterDuration)
{
    if (newNewlineAfterDuration == m_newlineAfterDuration)
        return;
    m_newlineAfterDuration = newNewlineAfterDuration;
}

bool HistoryModel::newlineAfterDurationEnabled() const
{
    return m_newlineAfterDuraionEnabled;
}

void HistoryModel::setNewlineAfterDurationEnabled(bool newNewlineAfterDuraionEnabled)
{
    if (newNewlineAfterDuraionEnabled == m_newlineAfterDuraionEnabled)
        return;
    m_newlineAfterDuraionEnabled = newNewlineAfterDuraionEnabled;
}

int HistoryModel::historyCapacity() const
{
    return m_historyCapacity;
}

QList<QByteArray> HistoryModel::splitDataByLength(const QByteArray &_data, int _chunkLength, int _firstChunkLength)
{
    Q_ASSERT_X(_chunkLength > 0, "split", "chunkLength cannot be zero");
    Q_ASSERT_X(_firstChunkLength <= _chunkLength, "split", "firstChunkLength cannot be greater than chunkLength");

    QList<QByteArray> newItems {};
    // add first chunk
    const int originalLength = _data.length();
    newItems.append(_data.left(std::min(originalLength, _firstChunkLength)));
    int from = _firstChunkLength;

    // add remaining chunks
    while (from < originalLength) {
        newItems.append(_data.mid(from, std::min(_chunkLength, originalLength - from)));
        from += _chunkLength;
    }

    return newItems;
}

QList<QByteArray> HistoryModel::splitData(const QByteArray &_data, bool limitByLength, int _chunkLength, int _firstChunkLength)
{
    if (limitByLength) {
        Q_ASSERT(_chunkLength > 0);
        Q_ASSERT(_firstChunkLength <= _chunkLength);

        if (_firstChunkLength == -1) _firstChunkLength = _chunkLength;
    }

    auto tmpData = _data.split('\n');
    QList<QByteArray> result {};

    Q_ASSERT(tmpData.length() > 0);

    if (tmpData.front().length() <= _firstChunkLength) {
        result.append(tmpData.front());
        tmpData.pop_front();
    } else {
        result.append(tmpData.front().left(_firstChunkLength));
        tmpData.front().remove(0, _firstChunkLength);
    }

    for (const auto &line : tmpData) {
        result.append(splitDataByLength(line, _chunkLength, _chunkLength));
    }

    return result;
}

QDateTime HistoryModel::now()
{
    return QDateTime::currentDateTime();
}
