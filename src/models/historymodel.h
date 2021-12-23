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
        TimestampRole,
        DirectionRole,
        HexRole,
        StringRole,
        NumColumns
    };

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
    void appendData(DataDirection _dir, const QByteArray &_data);

    int newlineAfterCount() const;
    void setNewlineAfterCount(int newNewlineAfterCount);

    bool newLineAfterCountEnabled() const;
    void setNewlineAfterCountEnabled(bool newNewLineAfterCountEnabled);

    int historyCapacity() const;

private:
    static bool isTextConvertible(const QByteArray &_data);
    QString formattedHexString(const QByteArray &_data) const;
    QString formattedString(const QByteArray &_data) const;
    static const char* toString(const DataDirection _dir);

signals:

private slots:

private:
    struct LogData {
        int index;
        DataDirection direction;
        QDateTime datetime;
        QByteArray data;
    };

    int m_totalLines {};
    int m_historyCapacity {};
    DataDirection m_lastDataDirection {};
    bool m_endedAtNewline {true};
    QList<LogData> m_items {};
    int m_newlineAfterCount {};
    bool m_newLineAfterCountEnabled {};
};

#endif // HISTORYMODEL_H
