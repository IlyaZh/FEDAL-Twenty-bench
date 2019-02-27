#ifndef SERIALPORTHANDLER_H
#define SERIALPORTHANDLER_H

//#include <globals.h>

#include <QObject>
#include <QString>
#include <QSerialPort>
#include <QTimer>
#include <QList>
#include <QQueue>

#include <QDebug>
#include <QDateTime>

class SerialPortHandler : public QObject
{
    Q_OBJECT
public:
    explicit SerialPortHandler(QString port, int baud = QSerialPort::Baud9600, int timeout = 100, QObject *parent = nullptr);
    ~SerialPortHandler();

    QString getPortName();
    int getBaudRate();
    bool isOpen();
    QString errorString();
    int getTimeout();
    void setTimeout(int value = 50);
    bool isBusy();
    void nextTask();
    qint8 getAddress();
    quint16 getRegister();
    quint8 getCount();
    QList<quint16> &getValues();
    int bytesToSend;
    bool queueIsEmpty();


    enum {READ_COMMAND = 0x03, WRITE_COMMAND = 0x06} modBusCommands;
    enum {ADDRESS_SHIFT = 0, FUNCTION_CODE_SHIFT = 1, REG_HIGH_SHIFT = 2, REG_LOW_SHIFT = 3, READ_COUNT_SHIFT = 2, WRITE_HIGH_DATA_SHIFT = 4, WRITE_LOW_DATA_SHIFT = 5, READ_HIGH_DATA_SHIFT = 3, READ_LOW_DATA_SHIFT = 4} modBusShift;

private:
    typedef struct {
        bool isRead;
        qint8 addr;
        quint16 reg;
        quint16 val;
    } reply_t;
    QSerialPort* serialPort;
    QByteArray buffer;
    quint16 crc16(const QByteArray &ptr, int size);
    QTimer* errorTimer;
    int expectedLength;
    qint8 addr;
    quint8 length;
    quint16 firstRegister;
    quint8 funcCode;
    QString errorMsg;
    QByteArray sentData;
    int timeout_period;
    QList<quint16> values;
    void startTransmit(QByteArray);
//    bool isTimeout;
    bool busyFlag;
    QQueue<reply_t> queue;
    void prepareToWrite();
    void prepareToRead();

signals:
    void newDataIsReady();
    void stateChanged(bool);
    void errorOccuredSignal();
    void timeoutSignal(quint8 address);
//    void portIsReady();

public slots:
    void setPort(QString);
    void setBaud(int);
    void setOpenState(bool);
    void dataToWrite(quint8 address, quint16 startRegister, quint16 value);
    void dataToRead(quint8 address, quint16 startRegister, quint8 count);
    void clearQueue();

private slots:
    void readyRead();
    void errorOccuredSlot(QSerialPort::SerialPortError error);
    void dataIsWritten(qint64);
    void timeOut();
    void clearBuffer();
};

#endif // SERIALPORTHANDLER_H
