#ifndef GLOBALS_H
#define GLOBALS_H

#include <QString>
#include <QLocale>
#include <QDir>

//#define DEBUG

const QString ORG_NAME = "FEDAL";
const QString APP_NAME = "Twenty Bench";
const QString PROGRAM_NAME = "Fedal Work Bench";
const int MAJOR_VERSION = 1;
const int MINOR_VERSION = 1;
const int PATCH_VERSION = 1;
const QString APP_TITLE = PROGRAM_NAME + " (Version " + QString::number(MAJOR_VERSION) + "." + QString::number(MINOR_VERSION) + "." + QString::number(PATCH_VERSION) + ")";
const QList<quint32> BAUDRATES = {9600, 14400, 19200};
extern QLocale wlocale;

const char DOUBLE_FORMAT = 'f';

const int CURRENT_FRACTIAL_SYMBOLS = 1;
const int DURATION_FRACTIAL_SYMBOLS = 0;
const int FREQUENCY_FRACTIAL_SYMBOLS = 1;
const int VOLTAGE_FRACTIAL_SYMBOLS = 1;
const int DELAY_FRACTIAL_SYMBOLS = 0;
const QString CURRENT_UNIT = "A";
const QString DURATION_UNIT = "мкс";
const QString FREQUENCY_UNIT = "Гц";
const QString VOLTAGE_UNIT = "В";
const QString DELAY_UNIT = "мкс";

const int STATUSBAR_MESSAGE_TIMEOUT = 5000; //ms
const int START_ADDRESS = 1;
const int DEVICE_COUNT = 20;
#ifdef DEBUG
const int COM_TIMEOUT = 2000; // ms
#else
const int COM_TIMEOUT = 100; // ms
#endif
const QList<quint16> readRegisters = {0x0011, 0x0021, 0x0001};
const QList<quint8> countRegisters = {5, 5, 7};
enum {MIN_REGS_SHIFT = 0, MAX_REGS_SHIFT = 1, VALUES_REGS_SHIFT = 2} arrayRegShift_t;

const QString LOG_FILE = QDir::homePath() + "/FEDAL20-errors.log";
const int MAX_LOG_FILE_SIZE = 1024*1024*100;

#endif // GLOBALS_H
