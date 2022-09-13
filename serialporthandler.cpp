#include "serialporthandler.h"

#include "crctable.h"
#include "globals.h"
#ifdef USING_SERIAL_MOCK
#include "mocks/serial_mock.hpp"
#endif

SerialPortHandler::SerialPortHandler(int tout, QObject* parent)
    : QThread(parent), dataThread(new models::DataThread) {
  setTimeout(tout);

  values.clear();
  buffer.clear();

  connect(dataThread, &models::DataThread::signal_timeout, this,
          &SerialPortHandler::timeOut);
  connect(dataThread, &models::DataThread::signal_errorOccured, this,
          [this](const QString& error) {
            emit signal_errorOccured(error);
            clearBuffer();
          });
  connect(dataThread, &models::DataThread::signal_receivedData, this,
          &SerialPortHandler::readyRead);
  connect(dataThread, &models::DataThread::signal_stateChanged, this,
          &SerialPortHandler::signal_stateChanged);
}

SerialPortHandler::~SerialPortHandler() { dataThread->disable(); }

bool SerialPortHandler::isOpen() { return dataThread->isOpen(); }

void SerialPortHandler::readyRead(const QByteArray& received_package) {
  buffer.append(received_package);
  emit signal_appendToLog("<- receive:" + buffer.toHex(' '));

  if (buffer.size() == expectedLength) {
    quint16 inCRC;
    inCRC = (static_cast<quint8>(buffer.at(expectedLength - 2)) << 8) |
            (static_cast<quint8>(buffer.at(expectedLength - 1)));
    if (inCRC == crc16(buffer, buffer.size() - 2)) {
      buffer = buffer.remove(expectedLength - 2, 2);

      values.clear();
      funcCode = static_cast<quint8>(buffer.at(FUNCTION_CODE_SHIFT));
      addr = static_cast<quint8>(buffer.at(ADDRESS_SHIFT));

      switch (funcCode) {
        case READ_COMMAND:
          length = static_cast<quint8>(buffer.at(READ_COUNT_SHIFT)) / 2;
          for (int i = 0; i < length; i++) {
            quint16 res =
                (static_cast<quint8>(buffer.at(READ_HIGH_DATA_SHIFT + 2 * i))
                 << 8) |
                static_cast<quint8>(buffer.at(READ_LOW_DATA_SHIFT + 2 * i));
            values.append(res);
          }
          break;
        case WRITE_COMMAND:
          firstRegister =
              (buffer.at(REG_HIGH_SHIFT) << 8) | (buffer.at(REG_LOW_SHIFT));
          for (int i = 0; i < length; i++)
            values.append(
                (static_cast<quint8>(buffer.at(WRITE_HIGH_DATA_SHIFT + 2 * i))
                 << 8) |
                static_cast<quint8>(buffer.at(WRITE_LOW_DATA_SHIFT + 2 * i)));
          break;
        default:
          emit signal_errorOccured("Incorrecet function code. Buffer: " +
                                   QString(buffer.toHex()));
          buffer.clear();
          return;
      }
      qInfo() << "<- receive:" << buffer.toHex(' ');
      emit signal_newDataIsReady();
    } else {
      emit signal_errorOccured("CRC is not valid! Buffer: " +
                               QString(buffer.toHex()));
    }
    buffer.clear();
  }
}

QList<quint16>& SerialPortHandler::getValues() { return values; }

void SerialPortHandler::setOpenState(bool state,
                                     std::optional<models::PortSettings> settings) {
  if (state == dataThread->isOpen()) return;

  if (dataThread->isOpen()) {
    dataThread->disable();
    clearQueue();
    clearBuffer();
  } else {
    dataThread->enable(settings.value());
  }
}

void SerialPortHandler::startTransmit(QByteArray&& str) {
  // передача в порт
  dataThread->getQueue()->push(str);
  qInfo() << " ";
  QString msg("-> push to queue: " + str.toHex(' '));
  qDebug() << msg;
  emit signal_appendToLog(msg);
}

void SerialPortHandler::prepareToWrite(reply_t&& reply) {
  buffer.clear();

  QByteArray msg;
  addr = reply.addr;
  firstRegister = reply.reg;
  funcCode = WRITE_COMMAND;
  length = 1;

  msg.append(reply.addr);
  msg.append(funcCode);
  msg.append(reply.reg >> 8);
  msg.append(reply.reg);
  msg.append(reply.val >> 8);
  msg.append(reply.val);
  quint16 crc = crc16(msg, msg.size());
  msg.append(static_cast<quint8>(crc >> 8));
  msg.append(static_cast<quint8>(crc));

  // for write command expected length is equal to written length
  expectedLength = msg.size();

  startTransmit(std::move(msg));
}

void SerialPortHandler::dataToWrite(quint8 address, quint16 startRegister,
                                    quint16 value) {
  if (!dataThread->isOpen()) return;

  reply_t reqData;
  reqData.isRead = false;
  reqData.addr = address;
  reqData.reg = startRegister;
  reqData.val = value;

  prepareToWrite(std::move(reqData));
}

void SerialPortHandler::prepareToRead(reply_t&& reply) {
  buffer.clear();
  QByteArray msg;
  addr = reply.addr;
  funcCode = READ_COMMAND;
  firstRegister = reply.reg;
  length = reply.val;

  msg.append(reply.addr);
  msg.append(funcCode);
  msg.append(static_cast<quint8>(reply.reg >> 8));
  msg.append(static_cast<quint8>(reply.reg));
  msg.append(static_cast<quint8>(reply.val >> 8));
  msg.append(static_cast<quint8>(reply.val));
  quint16 crc = crc16(msg, msg.size());
  msg.append(static_cast<quint8>(crc >> 8));
  msg.append(static_cast<quint8>(crc));

  // for write command expected length is equal to written length
  expectedLength = 2 * reply.val + 2 + 3;
  startTransmit(std::move(msg));
}

void SerialPortHandler::dataToRead(quint8 address, quint16 startRegister,
                                   quint8 count) {
  if (!dataThread->isOpen()) return;

  reply_t reqData;
  reqData.isRead = true;
  reqData.addr = address;
  reqData.reg = startRegister;
  reqData.val = count;

  prepareToRead(std::move(reqData));
}

quint16 SerialPortHandler::crc16(const QByteArray& ptr, int size) {
  uchar ucCRCHi = 0xFF;
  uchar ucCRCLo = 0xFF;
  int idx;

  for (int i = 0; i < size; i++) {
    uchar currByte = ptr.at(i);
    idx = ucCRCLo ^ currByte;
    ucCRCLo = (uchar)(ucCRCHi ^ aucCRCHi[idx]);
    ucCRCHi = aucCRCLo[idx];
  }

  return quint16(ucCRCLo << 8 | ucCRCHi);
}

void SerialPortHandler::timeOut() { emit signal_timeout(addr); }

void SerialPortHandler::setTimeout(int value) { dataThread->setTimeout(value); }

qint8 SerialPortHandler::getAddress() {
  if (addr < 0) addr = 0;
  return addr;
}

quint16 SerialPortHandler::getRegister() { return firstRegister; }

quint8 SerialPortHandler::getCount() { return length; }

// bool SerialPortHandler::queueIsEmpty() { return queue.isEmpty(); }

void SerialPortHandler::clearQueue() { dataThread->getQueue()->clear(); }

void SerialPortHandler::clearBuffer() {
  buffer.clear();
  addr = 1;
}
