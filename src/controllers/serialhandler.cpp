#include "serialhandler.h"
#include <QDebug>
#include <QMutexLocker>

SerialHandler::SerialHandler()
{
    qDebug();
    connect(this, &QSerialPort::readyRead, this, &SerialHandler::onReadyRead);
}

SerialHandler::~SerialHandler()
{
    if (isOpen()) {
        close();
    }
}

void SerialHandler::onReadyRead()
{
}

QString SerialHandler::formatData(const QByteArray &data)
{
    return data;
}
