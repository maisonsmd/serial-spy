#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QtSerialPort>
#include <QVector>
#include <QMenu>
#include "models/historymodel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_PROPERTY(int newlineAfterCount READ newlineAfterCount WRITE setNewlineAfterCount NOTIFY newlineAfterCountChanged)
    Q_PROPERTY(int newlineAfterDuration READ newlineAfterDuration WRITE setNewlineAfterDuration NOTIFY newlineAfterDurationChanged)
    Q_PROPERTY(bool newLineAfterCountEnabled READ newLineAfterCountEnabled WRITE setNewlineAfterCountEnabled NOTIFY newlineAfterCountEnabledChanged)
    Q_PROPERTY(bool newlineAfterDurationEnabled READ newlineAfterDurationEnabled WRITE setNewlineAfterDurationEnabled NOTIFY newlineAfterDurationEnabledChanged)
    Q_PROPERTY(bool dataLinkEnabled READ dataLinkEnabled WRITE setDataLinkEnabled NOTIFY dataLinkEnabledChanged)
    Q_PROPERTY(bool signalLinkEnabled READ signalLinkEnabled WRITE setSignalLinkEnabled NOTIFY signalLinkEnabledChanged)
    Q_PROPERTY(bool autoscroll READ autoscroll WRITE setAutoscroll NOTIFY autoscrollChanged)
    Q_PROPERTY(bool showTimestamp READ showTimestamp WRITE setShowTimestamp NOTIFY showTimestampChanged)
    Q_PROPERTY(bool showHexa READ showHexa WRITE setShowHexa NOTIFY showHexaChanged)
    Q_PROPERTY(int historyCapacity READ historyCapacity WRITE setHistoryCapacity NOTIFY historyCapacityChanged)

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    int newlineAfterCount() const;
    void setNewlineAfterCount(int newNewlineAfterCount);

    int newlineAfterDuration() const;
    void setNewlineAfterDuration(int newNewlineAfterDuration);

    bool newLineAfterCountEnabled() const;
    void setNewlineAfterCountEnabled(bool newNewlineAfterCountEnabled);

    bool newlineAfterDurationEnabled() const;
    void setNewlineAfterDurationEnabled(bool newNewlineAfterDuraionEnabled);

    bool dataLinkEnabled() const;
    void setDataLinkEnabled(bool newDataLinkEnabled);

    bool signalLinkEnabled() const;
    void setSignalLinkEnabled(bool newSignalLinkEnabled);

    bool autoscroll() const;
    void setAutoscroll(bool newAutoscroll);

    bool showTimestamp() const;
    void setShowTimestamp(bool newShowTimestamp);

    bool showHexa() const;
    void setShowHexa(bool newShowHexa);

    int historyCapacity() const;
    void setHistoryCapacity(int newHistoryCapacity);

private:
    void recalculateCellWidth();
    void setupActionMenu();
    void connectSignalSlots();

signals:
    void newlineAfterCountChanged();
    void newlineAfterDurationChanged();
    void newlineAfterCountEnabledChanged();
    void newlineAfterDurationEnabledChanged();
    void dataLinkEnabledChanged();
    void signalLinkEnabledChanged();
    void autoscrollChanged();
    void showTimestampChanged();
    void showHexaChanged();
    void historyCapacityChanged();

private slots:
    void onPortADataReceived();
    void onDataReceived(const QByteArray &_data);
    void onTableContextMenuRequested(const QPoint &_pos);

    void resizeToFit();
    void clearHistory();

private:
    Ui::MainWindow *ui;
    HistoryModel m_history {};
    QMenu m_tableContextMenu {this};

    QSerialPort m_portA {};
    QSerialPort *m_lastReceivedPort {nullptr};

    int m_newlineAfterCount {}; // bytes
    int m_newlineAfterDuration {}; // ms

    bool m_endedAtNewline {true};
    bool m_newlineAfterCountEnabled {};
    bool m_newlineAfterDuraionEnabled {};
    bool m_dataLinkEnabled {};
    bool m_signalLinkEnabled {};
    bool m_autoscroll {false};
    bool m_showTimestamp {true};
    bool m_showHexa {true};
    int m_historyCapacity {};
};
#endif // MAINWINDOW_H
