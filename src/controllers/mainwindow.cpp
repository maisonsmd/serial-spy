#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtDebug>
#include <QClipboard>
#include <algorithm>
// #include <QFontMetrics>
#include "utils/commonconfig.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(&m_portA, &QSerialPort::readyRead, this, &MainWindow::onPortADataReceived);

    setupActionMenu();

    ui->historyTable->setFont(QFont("Consolas"));
    ui->historyTable->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->historyTable->setModel(&m_history);
    ui->txtNewlineAfterBytes->setValidator(new QIntValidator(8, 1000, this));
    ui->txtNewlineAfterDuration->setValidator(new QIntValidator(10, 10000, this));
    ui->txtHistoryCap->setValidator(new QIntValidator(10, 999999, this));

    connectSignalSlots();

    // m_portA.setPortName("COM7");
    // m_portA.setBaudRate(1200);
    // qDebug() << m_portA.open(QIODevice::ReadWrite);

    setNewlineAfterCount(16);
    setNewlineAfterCountEnabled(true);

    setNewlineAfterDuration(500);
    setNewlineAfterDurationEnabled(true);

    setHistoryCapacity(10000);

    auto testTimer = new QTimer();
    connect(testTimer, &QTimer::timeout, this, [&]{
        auto data = QByteArray();
        data.reserve(20);
        for (int i = 0; i < 20; ++i) {
            data.append(rand());
        }
        onDataReceived(data);
        onDataReceived("1234567890");
    });

    testTimer->setInterval(500);
    testTimer->setSingleShot(false);
    onDataReceived("1234567890123456");
    // testTimer->start();
    onDataReceived("123456\n7822345678323456789423456789");
}

MainWindow::~MainWindow()
{
    qDebug("quit");
    delete ui;
}

void MainWindow::onPortADataReceived()
{
    m_lastReceivedPort = &m_portA;
    onDataReceived(m_portA.readAll());
}

void MainWindow::onDataReceived(const QByteArray &_data)
{
    m_endedAtNewline = false;

    auto data = _data;
    data = data.replace("\\n", "\n");
    data = data.replace("\\r", "\r");

    m_history.appendData(HistoryModel::A_TO_B, data);
    // resizeToFit();

    if (autoscroll()) {
        ui->historyTable->scrollToBottom();
    }
}

void MainWindow::onTableContextMenuRequested(const QPoint &_pos)
{
    m_tableContextMenu.popup(ui->historyTable->viewport()->mapToGlobal(_pos));
}

void MainWindow::resizeToFit()
{
    ui->historyTable->resizeRowsToContents();
    ui->historyTable->resizeColumnsToContents();
}

void MainWindow::clearHistory()
{
    m_history.clear();
}

int MainWindow::newlineAfterCount() const
{
    return m_newlineAfterCount;
}

void MainWindow::setNewlineAfterCount(int newNewlineAfterCount)
{
    if (m_newlineAfterCount == newNewlineAfterCount)
        return;

    if (newNewlineAfterCount <= 0)
        return;

    m_newlineAfterCount = newNewlineAfterCount;
    m_history.setNewlineAfterCount(newNewlineAfterCount);

    if (newNewlineAfterCount != ui->txtNewlineAfterBytes->text().toUInt())
        ui->txtNewlineAfterBytes->setText(QString::number(newNewlineAfterCount));

    // resizeToFit();

    emit newlineAfterCountChanged();
}

int MainWindow::newlineAfterDuration() const
{
    return m_newlineAfterDuration;
}

void MainWindow::setNewlineAfterDuration(int newNewlineAfterDuration)
{
    if (m_newlineAfterDuration == newNewlineAfterDuration)
        return;
    if (newNewlineAfterDuration <= 0)
        return;

    m_newlineAfterDuration = newNewlineAfterDuration;
    m_history.setNewlineAfterDuration(newNewlineAfterDuration);

    if (newNewlineAfterDuration != ui->txtNewlineAfterDuration->text().toUInt())
        ui->txtNewlineAfterDuration->setText(QString::number(newNewlineAfterDuration));

    emit newlineAfterDurationChanged();
}

bool MainWindow::newLineAfterCountEnabled() const
{
    return m_newlineAfterCountEnabled;
}

void MainWindow::setNewlineAfterCountEnabled(bool newNewlineAfterCountEnabled)
{
    qDebug() << newNewlineAfterCountEnabled;
    if (m_newlineAfterCountEnabled == newNewlineAfterCountEnabled)
        return;
    m_newlineAfterCountEnabled = newNewlineAfterCountEnabled;
    m_history.setNewlineAfterCountEnabled(newNewlineAfterCountEnabled);

    if (newNewlineAfterCountEnabled != ui->cbNewlineAfterBytes->isChecked())
        ui->cbNewlineAfterBytes->setChecked(newNewlineAfterCountEnabled);

    if (newNewlineAfterCountEnabled) {
        // resizeToFit();
    }

    emit newlineAfterCountEnabledChanged();
}

bool MainWindow::newlineAfterDurationEnabled() const
{
    return m_newlineAfterDuraionEnabled;
}

void MainWindow::setNewlineAfterDurationEnabled(bool newNewlineAfterDuraionEnabled)
{
    if (m_newlineAfterDuraionEnabled == newNewlineAfterDuraionEnabled)
        return;
    m_newlineAfterDuraionEnabled = newNewlineAfterDuraionEnabled;
    m_history.setNewlineAfterDurationEnabled(newNewlineAfterDuraionEnabled);

    if (newNewlineAfterDuraionEnabled != ui->cbNewlineAfterDuration->isChecked())
        ui->cbNewlineAfterDuration->setChecked(newNewlineAfterDuraionEnabled);

    emit newlineAfterDurationEnabledChanged();
}

bool MainWindow::dataLinkEnabled() const
{
    return m_dataLinkEnabled;
}

void MainWindow::setDataLinkEnabled(bool newDataLinkEnabled)
{
    if (m_dataLinkEnabled == newDataLinkEnabled)
        return;
    m_dataLinkEnabled = newDataLinkEnabled;
    emit dataLinkEnabledChanged();
}

bool MainWindow::signalLinkEnabled() const
{
    return m_signalLinkEnabled;
}

void MainWindow::setSignalLinkEnabled(bool newSignalLinkEnabled)
{
    if (m_signalLinkEnabled == newSignalLinkEnabled)
        return;
    m_signalLinkEnabled = newSignalLinkEnabled;
    emit signalLinkEnabledChanged();
}

bool MainWindow::autoscroll() const
{
    return m_autoscroll;
}

void MainWindow::setAutoscroll(bool newAutoscroll)
{
    if (m_autoscroll == newAutoscroll)
        return;
    m_autoscroll = newAutoscroll;

    if (newAutoscroll != ui->cbAutoScroll->isChecked()) {
        ui->cbAutoScroll->setChecked(newAutoscroll);
    }

    if (newAutoscroll) {
        ui->historyTable->scrollToBottom();
    }

    emit autoscrollChanged();
}

bool MainWindow::showTimestamp() const
{
    return m_showTimestamp;
}

void MainWindow::setShowTimestamp(bool newShowTimestamp)
{
    if (m_showTimestamp == newShowTimestamp)
        return;
    m_showTimestamp = newShowTimestamp;
    emit showTimestampChanged();
}

bool MainWindow::showHexa() const
{
    return m_showHexa;
}

void MainWindow::setShowHexa(bool newShowHexa)
{
    if (m_showHexa == newShowHexa)
        return;
    m_showHexa = newShowHexa;
    emit showHexaChanged();
}

int MainWindow::historyCapacity() const
{
    return m_historyCapacity;
}

void MainWindow::setHistoryCapacity(int newHistoryCapacity)
{
    if (m_historyCapacity == newHistoryCapacity)
        return;
    m_historyCapacity = newHistoryCapacity;
    m_history.setHistoryCapacity(newHistoryCapacity);

    if (newHistoryCapacity != ui->txtHistoryCap->text().toUInt()) {
        ui->txtHistoryCap->setText(QString::number(newHistoryCapacity));
    }

    emit historyCapacityChanged();
}

void MainWindow::setupActionMenu()
{
    m_tableContextMenu.addAction(ui->actResizeToFit);
    m_tableContextMenu.addAction(ui->actClearHistory);
    m_tableContextMenu.addAction(ui->actCopySelection);
    m_tableContextMenu.addAction(ui->actCopySelectionPayload);
    m_tableContextMenu.addAction(ui->actShowHexa);

    connect(ui->actResizeToFit, &QAction::triggered, this, &MainWindow::resizeToFit);
    // toggle HEX visibilily
    connect(ui->actShowHexa, &QAction::toggled, this, [&](){
        ui->historyTable->setColumnHidden(HistoryModel::toColumn(HistoryModel::HexRole), !ui->actShowHexa->isChecked());
    });
    connect(ui->actClearHistory, &QAction::triggered, this, &MainWindow::clearHistory);
    connect(ui->actCopySelection, &QAction::triggered, this, [&](){
        QString outputString {};
        int columnIndex {0};

        for (const auto &i : ui->historyTable->selectionModel()->selectedIndexes()) {
            outputString.append(i.data().toString().replace("\n", ""));

            // append '~' if not last column
            if (columnIndex != HistoryModel::toColumn(HistoryModel::ColumnRoles::NumColumns) - 1) {
                outputString.append(" ~ ");
                columnIndex++;
            } else {
                outputString.append("\n");
                columnIndex = 0;
            }
        }
        QApplication::clipboard()->setText(outputString);
    });
    connect(ui->actCopySelectionPayload, &QAction::triggered, this, [&](){
        QString outputString {};
        for (const auto &i : ui->historyTable->selectionModel()->selectedRows(HistoryModel::toColumn(HistoryModel::StringRole))) {
            outputString.append(i.data().toString());
            outputString.append("\n");
        }
        QApplication::clipboard()->setText(outputString);
    });
}

void MainWindow::connectSignalSlots()
{
    // show context menu
    connect(ui->historyTable, &QTableView::customContextMenuRequested, this, &MainWindow::onTableContextMenuRequested);

    // toggle newline
    connect(ui->cbNewlineAfterBytes, &QCheckBox::toggled, this, [&](){
        setNewlineAfterCountEnabled(ui->cbNewlineAfterBytes->isChecked());
    });
    connect(ui->txtNewlineAfterBytes, &QLineEdit::returnPressed, this, [&](){
        setNewlineAfterCount(ui->txtNewlineAfterBytes->text().toUInt());
    });
    connect(ui->cbNewlineAfterDuration, &QCheckBox::toggled, this, [&](){
        setNewlineAfterDurationEnabled(ui->cbNewlineAfterDuration->isChecked());
    });
    connect(ui->txtNewlineAfterDuration, &QLineEdit::returnPressed, this, [&](){
        setNewlineAfterDuration(ui->txtNewlineAfterDuration->text().toUInt());
    });

    // history capacity
    connect(ui->txtHistoryCap, &QLineEdit::returnPressed, this, [&](){
        setHistoryCapacity(ui->txtHistoryCap->text().toUInt());
    });
    // set autoscroll
    connect(ui->cbAutoScroll, &QCheckBox::toggled, this, [&](){
        setAutoscroll(ui->cbAutoScroll->isChecked());
    });

    // stop autoscroll when clicked on a row
    connect(ui->historyTable, &QTableView::clicked, this, [&](const QModelIndex &_index){
        Q_UNUSED(_index);
        setAutoscroll(false);
    });

    connect(ui->btnSendA, &QPushButton::released, this, [&](){
        onDataReceived(ui->cbbInputA->currentText().toLatin1());
    });
    connect(ui->cbbInputA->lineEdit(), &QLineEdit::returnPressed, this, [&](){
        onDataReceived(ui->cbbInputA->currentText().toLatin1());
    });
}
