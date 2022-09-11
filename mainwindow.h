#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QCloseEvent>
#include <QDebug>
#include <QMainWindow>
#include <QMessageBox>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QSettings>
#include <QTimer>
#include <QVBoxLayout>

#include "devicecontrol.h"
#include "globals.h"
#include "serialporthandler.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget* parent = nullptr);
  ~MainWindow();
  void closeEvent(QCloseEvent* event);

 private:
  Ui::MainWindow* ui;
  SerialPortHandler* serialPort;
  void setupWindow();
  void setDevParam(quint8 address, quint16 reg, quint16 value);
  QList<DeviceControl*> devices;
  void setupDevices(bool);
  qint8 currentAddress;
  int currentCommandId;
  bool paramsIsLoaded[2 * DEVICE_COUNT];
  void requestNextParam();
  QSettings* settings;
  void readSettings();
  void writeSettings();
  bool maybeSave();
  bool debugMode;

 private slots:
  void showError(const QString& msg);
  void openSettingsWindow(quint8 address);
  void setNewMaster(quint8, bool);
  void setNewDevName(quint8, QString);
  void onStateChanged(bool);
  void readReady();
  void onTimeout(quint8);
  void on_refreshPortsButton_clicked();
  void on_connectButton_clicked();
  //    void prepareDataToWrite(quint8, quint16, quint16);
  void on_delaySpinBox_editingFinished();
  void on_lastAddressSpinBox_editingFinished();
  void on_firstAddressSpinBox_editingFinished();
  void appendToWLog(QString msg);
  void on_debugBox_toggled(bool checked);
};

#endif  // MAINWINDOW_H
