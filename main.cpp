#include <QApplication>
#include <QLibraryInfo>
#include <QTranslator>

#include "globals.h"
#include "mainwindow.h"

QLocale wlocale = QLocale(QLocale::system());
void messageToFile(QtMsgType type, const QMessageLogContext &context,
                   const QString &msg);

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
#ifndef USING_SERIAL_MOCK
  qInstallMessageHandler(messageToFile);
#endif
  QTranslator qtTranslator;
  bool translate_file_loaded = qtTranslator.load("qt_en", ":/");
  qDebug() << "traslate_file_loaded" << translate_file_loaded;
  bool translate_loaded = a.installTranslator(&qtTranslator);
  qDebug() << "translate_loaded" << translate_loaded;
  MainWindow w;
  w.show();

  return a.exec();
}

void messageToFile(QtMsgType type, const QMessageLogContext &context,
                   const QString &msg) {
  QFile file(LOG_FILE);

  if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
    return;
  }

  if (file.size() > MAX_LOG_FILE_SIZE) {
    file.resize(0);
  }

  QTextStream out(&file);

  switch (type) {
    case QtInfoMsg:
      out << QDateTime::currentDateTime().toString("dd.MM.yy hh:mm:ss:zzz")
          << " Info: " << msg << ",     " << context.file << endl;
      break;
    case QtWarningMsg:
      out << QDateTime::currentDateTime().toString("dd.MM.yy hh:mm:ss:zzz")
          << " Warning: " << msg << ",      " << context.file << endl;
      break;
    case QtCriticalMsg:
      out << QDateTime::currentDateTime().toString("dd.MM.yy hh:mm:ss:zzz")
          << " Critical: " << msg << ",  " << context.file << endl;
      break;
    case QtFatalMsg:
      out << QDateTime::currentDateTime().toString("dd.MM.yy hh:mm:ss:zzz")
          << " Fatal: " << msg << ",     " << context.file << endl;
      break;
    default:
    case QtDebugMsg:
      break;
  }
  file.close();
}
