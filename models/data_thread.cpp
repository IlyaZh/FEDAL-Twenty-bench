#include "data_thread.h"

#include "queue.hpp"

#ifdef USING_SERIAL_MOCK
#include "serial_mock.hpp"
#else
#include <QSerialPort>
#endif

DataThread::DataThread(qint64 rx_package_size, qint64 error_retry,
                       QObject* parent)
    : QThread(parent),
      q(new Queue),
      rx_wait_size(rx_package_size),
      retry_counts(error_retry) {}

DataThread::~DataThread() { q.clear(); }

qint64 DataThread::countRetries() { return retry_counts; }

qint64 DataThread::rxWaitForSize() { return rx_wait_size; }

QSharedPointer<Queue> DataThread::getQueue() { return q; }

bool DataThread::isOpen() const { return is_working.load(); }

void DataThread::setTimeout(qint64 timeout_ms) { timeout.store(timeout_ms); }

void DataThread::setDelay(qint64 delay_ms) { delay.store(delay_ms); }

void DataThread::disable() { is_working.store(false); }

void DataThread::enable(PortSettings& settings) {
  if (!isRunning()) {
    is_working.store(true);
    QThread::start(LowPriority);
    settings_ = settings;
  }
}

void DataThread::run() {
#ifdef USING_SERIAL_MOCK
  io = new SerialPort_Mock();
#else
  io = new QSerialPort();
#endif
  io->setBaudRate(settings_.baud_rate);
  io->setPortName(settings_.port_name);
  io->setStopBits(QSerialPort::TwoStop);
  io->setDataBits(QSerialPort::Data8);
  io->setParity(QSerialPort::NoParity);
  io->open(QIODevice::ReadWrite);
  if (!io->isOpen()) {
    emit signal_errorOccured("IO device hasn't opened");
    return;
  }
  emit signal_stateChanged(io->isOpen());

  if (q->empty()) {
    emit signal_readyToWrite();
  }
  connect(q.data(), &Queue::EmptyOccured, this,
          &DataThread::signal_readyToWrite);
  qDebug() << "Serial port has started";
  QByteArray rx_buffer;
  while (is_working.load()) {
    const auto t = timeout.load();
    if (!io->isOpen()) {
      emit signal_errorOccured(
          QString("IO device hasn't opened. Waiting for %1 ms timeout.")
              .arg(t));
      QThread::msleep(t);
    }

    auto package = q->pop();
    io->write(package);
    if (!io->waitForBytesWritten(t)) {
      emit signal_timeout(package);
      continue;
    }
    if (!io->waitForReadyRead(t)) {
      emit signal_timeout(package);
      continue;
    }
    auto received_package = io->readAll();
    rx_buffer.append(received_package);
    if (received_package.size() >= rx_wait_size) {
      emit signal_receivedData(rx_buffer);
      rx_buffer.clear();
    }
    QThread::msleep(delay.load());
  }
  q.clear();
  if (io->isOpen()) {
    io->close();
  }
  emit signal_stateChanged(io->isOpen());
  qDebug() << "Serial port thread has stopped";
  if (io != nullptr) {
    delete io;
  }
  io = nullptr;
}
