#ifndef HISTORYMODEL_H
#define HISTORYMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include <QDateTime>

class HistoryModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum ColumnRoles {
        TimestampRole = Qt::UserRole + 1,
        DirectionRole,
        HexRole,
        StringRole,
        NumColumns
    };

    static constexpr int toColumn(ColumnRoles role) {
        return role - TimestampRole;
    }

    static constexpr ColumnRoles toRole(int column) {
        return static_cast<ColumnRoles>(TimestampRole + column);
    }

    enum DataDirection {
        A_TO_B,
        B_TO_A,
        A_TO_PC,
        B_TO_PC,
        PC_TO_A,
        PC_TO_B
    };

    explicit HistoryModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Remove data:
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    void clear();
    void setHistoryCapacity(int _cap);
    void addItem(DataDirection _dir, const QByteArray &_data);
    void addItems(DataDirection _dir, const QList<QByteArray> &_data);
    void appendData(DataDirection _dir, const QByteArray &_data);

    int newlineAfterCount() const;
    void setNewlineAfterCount(int newNewlineAfterCount);

    bool newLineAfterCountEnabled() const;
    void setNewlineAfterCountEnabled(bool newNewLineAfterCountEnabled);

    int newlineAfterDuration() const;
    void setNewlineAfterDuration(int newNewlineAfterDuration);

    bool newlineAfterDurationEnabled() const;
    void setNewlineAfterDurationEnabled(bool newNewlineAfterDuraionEnabled);

    int historyCapacity() const;

private:
    QString formattedHexString(const QByteArray &_data) const;
    QString formattedString(const QByteArray &_data) const;
    static const char* toString(const DataDirection _dir);
    static QList<QByteArray> splitDataByLength(const QByteArray &_data, int _chunkLength, int _firstChunkLength);
    static QList<QByteArray> splitData(const QByteArray &_data, bool limitByLength, int _chunkLength = -1, int _firstChunkLength = -1);
    static QDateTime now();

signals:

private slots:

private:
    struct LogData {
        int index;
        DataDirection direction;
        QDateTime createdDatetime;
        QDateTime lastAppendDateTime;
        QByteArray data;
    };

    int m_totalLines {};
    int m_historyCapacity {};
    bool m_endedAtNewline {true};
    QList<LogData> m_items {};
    int m_newlineAfterCount {};
    bool m_newLineAfterCountEnabled {};
    int m_newlineAfterDuration {}; // ms
    bool m_newlineAfterDuraionEnabled {};
};

#endif // HISTORYMODEL_H
