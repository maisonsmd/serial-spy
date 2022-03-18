#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtDebug>
#include <QFontMetrics>
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

    connectSignalSlots();

    // m_portA.setPortName("COM7");
    // m_portA.setBaudRate(1200);
    // qDebug() << m_portA.open(QIODevice::ReadWrite);

    setNewlineAfterCount(16);
    setNewlineAfterCountEnabled(true);

    setNewlineAfterDuration(500);
    setNewlineAfterDurationEnabled(true);

    setHistoryCapacity(100);

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
    testTimer->start();
    onDataReceived("1234567822345678323456789423456789");
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

    m_history.appendData(HistoryModel::A_TO_B, _data);
    resizeToFit();

    if (autoscroll()) {
        ui->historyTable->scrollToBottom();
    }
}

void MainWindow::onTableContextMenuRequested(const QPoint &_pos)
{
    QModelIndex index = ui->historyTable->indexAt(_pos);
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

    // recalculateCellWidth();
    resizeToFit();

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
        recalculateCellWidth();
    }
    resizeToFit();

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
    emit historyCapacityChanged();
}

void MainWindow::recalculateCellWidth()
{
    QFontMetrics fm(ui->historyTable->font());
    int width {};
    constexpr int margin {10};

    // hex & string column
    if (newLineAfterCountEnabled() && newlineAfterCount() > 0) {
        width = fm.boundingRect(QByteArray(newlineAfterCount() * 3, '-')).width();
        ui->historyTable->setColumnWidth(HistoryModel::HexRole, width + margin);
        ui->historyTable->setColumnWidth(HistoryModel::StringRole, width / 3 + margin);
    }

    // timestamp column
    width = fm.boundingRect(TIME_FORMAT " ").width();
    ui->historyTable->setColumnWidth(HistoryModel::TimestampRole, width + margin);

    // direction column
    width = fm.boundingRect("A->B ").width();
    ui->historyTable->setColumnWidth(HistoryModel::DirectionRole, width + margin);
}

void MainWindow::setupActionMenu()
{
    m_tableContextMenu.addAction(ui->actionResize_to_fit);
    m_tableContextMenu.addAction(ui->action_Clear_history);
    m_tableContextMenu.addAction(ui->actionCopy_as_text);
    m_tableContextMenu.addAction(ui->actionCopy_payload_as_text);
    m_tableContextMenu.addAction(ui->actionShow_HEX_column);

    connect(ui->actionResize_to_fit, &QAction::triggered, this, &MainWindow::resizeToFit);
    // toggle HEX visibilily
    connect(ui->actionShow_HEX_column, &QAction::toggled, this, [&](){
        ui->historyTable->setColumnHidden(HistoryModel::ColumnRoles::HexRole, !ui->actionShow_HEX_column->isChecked());
    });
    connect(ui->action_Clear_history, &QAction::triggered, this, &MainWindow::clearHistory);
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
}
