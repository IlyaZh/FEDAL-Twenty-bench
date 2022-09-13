#ifndef SERIALPORTHANDLER_H
#define SERIALPORTHANDLER_H

#include <QDateTime>
#include <QDebug>
#include <QList>
#include <QMutex>
#include <QObject>
#include <QQueue>
#include <QSemaphore>
#include <QSerialPort>
#include <QString>
#include <QThread>
#include <QTimer>
#include <optional>

#include "models/data_thread.h"

#ifdef USING_SERIAL_MOCK
#include "mocks/serial_mock.hpp"
class SerialPort_Mock;
#endif

class SerialPortHandler : public QObject {
  Q_OBJECT
 public:
  explicit SerialPortHandler(int timeout = 50, QObject* parent = nullptr);
  ~SerialPortHandler();

  bool isOpen();
  void setTimeout(int value = 50);
  qint8 getAddress();
  quint16 getRegister();
  quint8 getCount();
  QList<quint16>& getValues();

  enum {
    MIN_REGS_SHIFT = 0,
    MAX_REGS_SHIFT = 1,
    VALUES_REGS_SHIFT = 2
  } arrayRegShift_t;
  enum { READ_COMMAND = 0x03, WRITE_COMMAND = 0x06 } modBusCommands;
  enum {
    ADDRESS_SHIFT = 0,
    FUNCTION_CODE_SHIFT = 1,
    REG_HIGH_SHIFT = 2,
    REG_LOW_SHIFT = 3,
    READ_COUNT_SHIFT = 2,
    WRITE_HIGH_DATA_SHIFT = 4,
    WRITE_LOW_DATA_SHIFT = 5,
    READ_HIGH_DATA_SHIFT = 3,
    READ_LOW_DATA_SHIFT = 4
  } modBusShift;

 private:
  typedef struct {
    bool isRead;
    qint8 addr;
    quint16 reg;
    quint16 val;
  } reply_t;
  QByteArray buffer;
  quint16 crc16(const QByteArray& ptr, int size);
  int expectedLength;
  qint8 addr;
  quint8 length;
  quint16 firstRegister;
  quint8 funcCode;
  QByteArray sentData;
  QList<quint16> values;
  void startTransmit(QByteArray&&);
  void prepareToWrite(reply_t&& reply);
  void prepareToRead(reply_t&& reply);
  models::DataThread* dataThread;

 signals:
  void signal_newDataIsReady();
  void signal_stateChanged(bool);
  void signal_errorOccured(const QString& error);
  void signal_timeout(quint8 address);
  void signal_appendToLog(QString);

 public slots:
  void setOpenState(bool, std::optional<models::PortSettings>);
  void dataToWrite(quint8 address, quint16 startRegister, quint16 value);
  void dataToRead(quint8 address, quint16 startRegister, quint8 count);
  void clearQueue();

 private slots:
  void readyRead(const QByteArray& received_package);
  void timeOut();
  void clearBuffer();
};

#endif  // SERIALPORTHANDLER_H
