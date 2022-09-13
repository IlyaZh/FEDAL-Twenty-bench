#pragma once

#include <QDir>
#include <QLocale>
#include <QString>

#ifdef QT_DEBUG
#define USING_SERIAL_MOCK
#endif

const QString ORG_NAME("FEDAL");
const QString APP_NAME("Twenty Bench");
const QString PROGRAM_NAME("Fedal Work Bench");
constexpr int MAJOR_VERSION = 1;
constexpr int MINOR_VERSION = 2;
constexpr int PATCH_VERSION = 0;
const QString APP_TITLE =
    PROGRAM_NAME + " (Version " + QString::number(MAJOR_VERSION) + "." +
    QString::number(MINOR_VERSION) + "." + QString::number(PATCH_VERSION) + ")";
const QList<quint32> BAUDRATES = {9600, 14400, 19200};
extern QLocale wlocale;

constexpr char DOUBLE_FORMAT = 'f';

constexpr int CURRENT_FRACTIAL_SYMBOLS = 1;
constexpr int DURATION_FRACTIAL_SYMBOLS = 0;
constexpr int FREQUENCY_FRACTIAL_SYMBOLS = 1;
constexpr int VOLTAGE_FRACTIAL_SYMBOLS = 1;
constexpr int DELAY_FRACTIAL_SYMBOLS = 0;
const QString CURRENT_UNIT = "A";
const QString DURATION_UNIT = "мкс";
const QString FREQUENCY_UNIT = "Гц";
const QString VOLTAGE_UNIT = "В";
const QString DELAY_UNIT = "мкс";

constexpr int STATUSBAR_MESSAGE_TIMEOUT = 5000;  // ms
constexpr int START_ADDRESS = 1;
constexpr int DEVICE_COUNT = 20;
constexpr int COM_TIMEOUT = 100;  // ms
const QList<quint16> readRegisters = {0x0011, 0x0021, 0x0001};
const QList<quint8> countRegisters = {5, 5, 7};

const QString LOG_FILE = QDir::homePath() + "/FEDAL20-errors.log";
constexpr int MAX_LOG_FILE_SIZE = 1024 * 1024 * 100;
