#ifndef SERIALHANDLER_H
#define SERIALHANDLER_H

#include <QtSerialPort/QtSerialPort>
#include <QByteArray>

// struct StaticBuffer {
    // size_t length;
    // uint8_t data[MAX_BUFFER_LENGTH];
// };

class SerialHandler : public QSerialPort
{
    Q_OBJECT
public:
    SerialHandler();
    ~SerialHandler();

private:
    QString formatData(const QByteArray &data);

signals:
    void dataReceived(const QString &data);

private slots:
    void onReadyRead();

private:
};

#endif // SERIALHANDLER_H
