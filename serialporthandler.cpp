#include "serialporthandler.h"
#include "crctable.h"
//#include "globals.h"

SerialPortHandler::SerialPortHandler(QString port, int baud, int tout, QObject *parent) : QObject(parent),
    serialPort(nullptr)
{
    serialPort = new QSerialPort();
    serialPort->setStopBits(QSerialPort::TwoStop);
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setParity(QSerialPort::NoParity);
    setBaud(baud);
    setPort(port);
    setTimeout(tout);

    timeout_period = 50;
    values.clear();
    buffer.clear();
    clearQueue();
    busyFlag = false;

    errorTimer = new QTimer();
    errorTimer->setSingleShot(true);
    errorTimer->setInterval(timeout_period);
    connect(errorTimer, SIGNAL(timeout()), this, SLOT(timeOut()));
    connect(serialPort, SIGNAL(bytesWritten(qint64)), this, SLOT(dataIsWritten(qint64)));
    connect(serialPort, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(this, SIGNAL(errorOccuredSignal()), this, SLOT(clearBuffer()));
}

SerialPortHandler::~SerialPortHandler() {
    serialPort->deleteLater();
}

bool SerialPortHandler::isOpen() {
    return serialPort->isOpen();
}

QString SerialPortHandler::errorString() {
    return errorMsg;
}

QString SerialPortHandler::getPortName() {
    return serialPort->portName();
}

int SerialPortHandler::getBaudRate() {
    return serialPort->baudRate();
}

void SerialPortHandler::setPort(QString port) {
    serialPort->setPortName(port);
}

void SerialPortHandler::setBaud(int baud) {
    serialPort->setBaudRate(baud);
}

void SerialPortHandler::readyRead() {
    errorTimer->stop();
    errorTimer->setInterval(timeout_period);

    buffer.append(serialPort->readAll());
    emit appendToLog("<- receive:" + buffer.toHex(' '));

    if(buffer.size() == expectedLength) {
        quint16 inCRC;
        inCRC = (static_cast<quint8>(buffer.at(expectedLength-2))<<8)|(static_cast<quint8>(buffer.at(expectedLength-1)));
        if(inCRC == crc16(buffer, buffer.size()-2)) {
            buffer = buffer.remove(expectedLength-2, 2);

            values.clear();
            funcCode = static_cast<quint8>(buffer.at(FUNCTION_CODE_SHIFT));
            addr = static_cast<quint8>(buffer.at(ADDRESS_SHIFT));

            switch(funcCode) {
            case READ_COMMAND:
                length = static_cast<quint8>(buffer.at(READ_COUNT_SHIFT))/2;
                for(int i =0; i < length; i++) {
                    quint16 res = (static_cast<quint8>(buffer.at(READ_HIGH_DATA_SHIFT+2*i))<<8) | static_cast<quint8>(buffer.at(READ_LOW_DATA_SHIFT+2*i));
                    values.append(res);
                }
                break;
            case WRITE_COMMAND:
                firstRegister = (buffer.at(REG_HIGH_SHIFT)<<8) | (buffer.at(REG_LOW_SHIFT));
                for (int i = 0; i < length; i++)
                    values.append((static_cast<quint8>(buffer.at(WRITE_HIGH_DATA_SHIFT+2*i))<<8) | static_cast<quint8>(buffer.at(WRITE_LOW_DATA_SHIFT+2*i)));
                break;
            default:
                errorMsg = "Incorrecet function code. Buffer: " + QString(buffer.toHex());
                emit errorOccuredSignal();
                buffer.clear();
                return;
            }
            qInfo() << "<- receive:" << buffer.toHex(' ');
            emit newDataIsReady();
        } else {
            errorMsg = "CRC is not valid! Buffer: " + QString(buffer.toHex());
            emit errorOccuredSignal();
        }
        buffer.clear();
    } else {
        errorTimer->stop();
        errorTimer->setInterval(timeout_period);
        errorTimer->start();
    }
}

QList<quint16>& SerialPortHandler::getValues() {
    return values;
}

void SerialPortHandler::setOpenState(bool state) {
    if(state == serialPort->isOpen()) return;

    if(serialPort->isOpen()) {
        serialPort->close();
        clearQueue();
        clearBuffer();
        emit stateChanged(serialPort->isOpen());
    } else {
        if(serialPort->open(QIODevice::ReadWrite)) {
            emit stateChanged(serialPort->isOpen());
        }
    }
}

void SerialPortHandler::errorOccuredSlot(QSerialPort::SerialPortError error) {
    if(error != QSerialPort::NoError) {
        errorMsg = serialPort->errorString();
//        clearQueue();
        emit errorOccuredSignal();
        serialPort->clearError();
    }
}

void SerialPortHandler::dataIsWritten(qint64 sizeWritten) {
    bytesToSend -= sizeWritten;
    errorTimer->stop();
    errorTimer->setInterval(timeout_period);
    errorTimer->start();
}

void SerialPortHandler::startTransmit(QByteArray str) {
    // передача в порт
    bytesToSend = str.size();
    sentData = str;
    bytesToSend -= serialPort->write(str, bytesToSend);
    qInfo() << " ";
    qInfo() << "-> transmit: " << str.toHex(' ');
    emit appendToLog("-> transmit: " + str.toHex(' '));
    errorTimer->stop();
    errorTimer->setInterval(timeout_period);
    errorTimer->start();
}

void SerialPortHandler::prepareToWrite() {
    busyFlag = true;

    reply_t qData = queue.dequeue();

    buffer.clear();

    QByteArray msg;
    addr = qData.addr;
    firstRegister = qData.reg;
    funcCode = WRITE_COMMAND;
    length = 1;

    msg.append(qData.addr);
    msg.append(funcCode);
    msg.append(qData.reg>>8);
    msg.append(qData.reg);
    msg.append(qData.val>>8);
    msg.append(qData.val);
    quint16 crc = crc16(msg, msg.size());
    msg.append(static_cast<quint8> (crc>>8));
    msg.append(static_cast<quint8> (crc));

    // for write command expected length is equal to written length
    expectedLength = msg.size();

    startTransmit(msg);
}

void SerialPortHandler::dataToWrite(quint8 address, quint16 startRegister, quint16 value) {
    if(!serialPort->isOpen()) return;

    reply_t reqData;
    reqData.isRead = false;
    reqData.addr = address;
    reqData.reg  = startRegister;
    reqData.val = value;

    queue.prepend(reqData);

    if(busyFlag) return;

    prepareToWrite();
}

void SerialPortHandler::prepareToRead() {
    busyFlag = true;
    reply_t qData = queue.dequeue();

    buffer.clear();
    QByteArray msg;
    addr = qData.addr;
    funcCode = READ_COMMAND;
    firstRegister = qData.reg;
    length = qData.val;

    msg.append(qData.addr);
    msg.append(funcCode);
    msg.append(static_cast<quint8> (qData.reg>>8));
    msg.append(static_cast<quint8> (qData.reg));
    msg.append(static_cast<quint8> (qData.val>>8));
    msg.append(static_cast<quint8> (qData.val));
    quint16 crc = crc16(msg, msg.size());
    msg.append(static_cast<quint8> (crc>>8));
    msg.append(static_cast<quint8> (crc));

    // for write command expected length is equal to written length
    expectedLength = 2*qData.val+2+3;
    startTransmit(msg);
}

void SerialPortHandler::dataToRead(quint8 address, quint16 startRegister, quint8 count) {
    if(!serialPort->isOpen()) return;

    reply_t reqData;
    reqData.isRead = true;
    reqData.addr = address;
    reqData.reg = startRegister;
    reqData.val = count;

    queue.append(reqData);

    if(busyFlag) return;

    prepareToRead();
}

quint16 SerialPortHandler::crc16(const QByteArray &ptr, int size) {
    uchar   ucCRCHi = 0xFF;
    uchar   ucCRCLo = 0xFF;
    int idx;

        for(int i =0; i < size; i++)
        {
            uchar currByte = ptr.at(i);
            idx = ucCRCLo ^ currByte;
            ucCRCLo = (uchar) ( ucCRCHi ^ aucCRCHi[idx] );
            ucCRCHi = aucCRCLo[idx];
        }

        return quint16 ( ucCRCLo << 8 | ucCRCHi );
}



void SerialPortHandler::timeOut() {
    busyFlag = false;
    errorMsg = "Timeout address is " + QString::number(addr);
    emit timeoutSignal(addr);
}

int SerialPortHandler::getTimeout() {
    return timeout_period;
}

void SerialPortHandler::setTimeout(int value) {
    timeout_period = value;
}

bool SerialPortHandler::isBusy() {
    return busyFlag;
}

void SerialPortHandler::nextTask() {
    busyFlag = false;
//    return; // for debug only. You'll delete this line before making poduction

    if(!queue.isEmpty()) {
        if(queue.head().isRead) {
            prepareToRead();
        } else {
            prepareToWrite();
        }
    }
}

qint8 SerialPortHandler::getAddress() {
    if(addr < 0) addr = 0;
    return addr;
}

quint16 SerialPortHandler::getRegister() {
    return firstRegister;
}

quint8 SerialPortHandler::getCount() {
    return length;
}

bool SerialPortHandler::queueIsEmpty() {
    return queue.isEmpty();
}

void SerialPortHandler::clearQueue() {
    queue.clear();
    busyFlag = false;
}

void SerialPortHandler::clearBuffer() {
    buffer.clear();
    busyFlag = false;
}
