#include "devicecontrol.h"

DeviceControl::DeviceControl(quint8 addr, QWidget* parent) : QWidget(parent) {
  address = addr;

  QUiLoader loader;
  QFile file(":/forms/devicecontrolform.ui");
  file.open(QFile::ReadOnly);
  mainWidget = loader.load(&file, this);
  file.close();

  ui_devNameLabel = mainWidget->findChild<QLabel*>("devNameLabel");
  ui_currentValueLabel = mainWidget->findChild<QLabel*>("currentLabel");
  ui_durationValueLabel = mainWidget->findChild<QLabel*>("durationLabel");
  ui_frequencyValueLabel = mainWidget->findChild<QLabel*>("frequencyLabel");
  ui_voltageValueLabel = mainWidget->findChild<QLabel*>("voltageLabel");
  ui_delayValueLabel = mainWidget->findChild<QLabel*>("delayLabel");
  ui_linkLabel = mainWidget->findChild<QLabel*>("linkLabel");

  ui_controlBox = mainWidget->findChild<QGroupBox*>("controlBox");
  ui_currentSpinBox = mainWidget->findChild<QDoubleSpinBox*>("currentSpinBox");
  ui_durationSpinBox =
      mainWidget->findChild<QDoubleSpinBox*>("durationSpinBox");
  ui_frequencySpinBox =
      mainWidget->findChild<QDoubleSpinBox*>("frequencySpinBox");
  ui_voltageSpinBox = mainWidget->findChild<QDoubleSpinBox*>("voltageSpinBox");
  ui_delaySpinBox = mainWidget->findChild<QDoubleSpinBox*>("delaySpinBox");
  ui_devNameEdit = mainWidget->findChild<QLineEdit*>("devNameEdit");

  ui_masterSlaveButton =
      mainWidget->findChild<QPushButton*>("masterSlaveButton");
  ui_powerButton = mainWidget->findChild<QPushButton*>("powerButton");
  ui_startStopButton = mainWidget->findChild<QPushButton*>("startStopButton");
  ui_settingsButton = mainWidget->findChild<QPushButton*>("settingsButton");
  ui_currentMaxLabel = mainWidget->findChild<QLabel*>("maxCurrentLabel");
  ui_durationMaxLabel = mainWidget->findChild<QLabel*>("maxDurationLabel");
  ui_frequencyMaxLabel = mainWidget->findChild<QLabel*>("maxFrequencyLabel");
  ui_voltageMaxLabel = mainWidget->findChild<QLabel*>("maxVoltageLabel");
  ui_delayMaxLabel = mainWidget->findChild<QLabel*>("maxDelayLabel");
  ui_currentMinLabel = mainWidget->findChild<QLabel*>("minCurrentLabel");
  ui_durationMinLabel = mainWidget->findChild<QLabel*>("minDurationLabel");
  ui_frequencyMinLabel = mainWidget->findChild<QLabel*>("minFrequencyLabel");
  ui_voltageMinLabel = mainWidget->findChild<QLabel*>("minVoltageLabel");
  ui_delayMinLabel = mainWidget->findChild<QLabel*>("minDelayLabel");

  mainWidget->setSizePolicy(QSizePolicy::MinimumExpanding,
                            QSizePolicy::Maximum);

  QMetaObject::connectSlotsByName(this);

  setLink(DeviceControl::NO_LINK_STATE);
  showControl(false);
  setSlaveButton(true);
  setPowerButton(false);

  ui_currentSpinBox->setDecimals(CURRENT_FRACTIAL_SYMBOLS);
  ui_durationSpinBox->setDecimals(DURATION_FRACTIAL_SYMBOLS);
  ui_frequencySpinBox->setDecimals(FREQUENCY_FRACTIAL_SYMBOLS);
  ui_voltageSpinBox->setDecimals(VOLTAGE_FRACTIAL_SYMBOLS);
  ui_delaySpinBox->setDecimals(DELAY_FRACTIAL_SYMBOLS);

  ui_currentSpinBox->setSingleStep(1.0 /
                                   qRound(pow(10, CURRENT_FRACTIAL_SYMBOLS)));
  ui_durationSpinBox->setSingleStep(1.0 /
                                    qRound(pow(10, DURATION_FRACTIAL_SYMBOLS)));
  ui_frequencySpinBox->setSingleStep(
      1.0 / qRound(pow(10, FREQUENCY_FRACTIAL_SYMBOLS)));
  ui_voltageSpinBox->setSingleStep(1.0 /
                                   qRound(pow(10, VOLTAGE_FRACTIAL_SYMBOLS)));
  ui_delaySpinBox->setSingleStep(1.0 / qRound(pow(10, DELAY_FRACTIAL_SYMBOLS)));
}

QWidget* DeviceControl::loadWidget() { return mainWidget; }

DeviceControl::~DeviceControl() { delete mainWidget; }

void DeviceControl::setCurrentValue(double value) {
  current = value;
  ui_currentValueLabel->setText(
      wlocale.toString(current, DOUBLE_FORMAT, CURRENT_FRACTIAL_SYMBOLS) +
      QString(" ") + CURRENT_UNIT);
  if (controlIsShown) return;
  ui_currentSpinBox->setValue(current);
}

void DeviceControl::setDurationValue(double value) {
  duration = value;
  ui_durationValueLabel->setText(
      wlocale.toString(value, DOUBLE_FORMAT, DURATION_FRACTIAL_SYMBOLS) +
      QString(" ") + DURATION_UNIT);
  if (controlIsShown) return;
  ui_durationSpinBox->setValue(duration);
}

void DeviceControl::setFrequencyValue(double value) {
  frequency = value;
  ui_frequencyValueLabel->setText(
      wlocale.toString(value, DOUBLE_FORMAT, FREQUENCY_FRACTIAL_SYMBOLS) +
      QString(" ") + FREQUENCY_UNIT);
  if (controlIsShown) return;
  ui_frequencySpinBox->setValue(value);
}

void DeviceControl::setVoltageValue(double value) {
  voltage = value;
  ui_voltageValueLabel->setText(
      wlocale.toString(value, DOUBLE_FORMAT, VOLTAGE_FRACTIAL_SYMBOLS) +
      QString(" ") + VOLTAGE_UNIT);
  if (controlIsShown) return;
  ui_voltageSpinBox->setValue(voltage);
}

void DeviceControl::setDelayValue(double value) {
  delay = value;
  ui_delayValueLabel->setText(
      QString("td: ") +
      wlocale.toString(value, DOUBLE_FORMAT, DELAY_FRACTIAL_SYMBOLS) +
      QString(" ") + DELAY_UNIT);
  if (controlIsShown) return;
  ui_delaySpinBox->setValue(delay);
}

void DeviceControl::setDevName(QString str) {
  ui_devNameLabel->setText(str);
  ui_devNameEdit->setText(str);
}

quint8 DeviceControl::getAddress() { return address; }

double DeviceControl::getCurrentValue() { return current; }

double DeviceControl::getFrequencyValue() { return frequency; }

double DeviceControl::getDurationValue() { return duration; }

double DeviceControl::getVoltageValue() { return voltage; }

double DeviceControl::getDelayValue() { return delay; }

QString DeviceControl::getDevName() { return ui_devNameLabel->text(); }

void DeviceControl::on_settingsButton_clicked(bool state) {
  emit showControlBoxSignal(address);
  showControl(state);
}

bool DeviceControl::getLink() { return link; }

void DeviceControl::setLink(linkState_t state) {
  switch (state) {
    case LINK_STATE:
      link = true;
      ui_linkLabel->setStyleSheet(
          "QLabel { \
                                    width: 20px; \
                                    height: 20px; \
                                    border-radius: 10px; \
                                    border: 1px solid rgb(0, 255, 0); \
                                    background-color: rgb(0, 255, 0); \
                                }");
      break;
    case NO_LINK_STATE:
      ui_linkLabel->setStyleSheet(
          "QLabel { \
                                    width: 20px; \
                                    height: 20px; \
                                    border-radius: 10px; \
                                    border: 1px solid white; \
                                    background-color: rgb(255, 255, 255); \
                                }");
      link = false;
      break;
    case ERROR_STATE:
      ui_linkLabel->setStyleSheet(
          "QLabel { \
                                    width: 20px; \
                                    height: 20px; \
                                    border-radius: 10px; \
                                    border: 1px solid red; \
                                    background-color: rgb(229, 34, 0); \
                                }");
      break;
  }
}

void DeviceControl::showControl(bool state) {
  controlIsShown = state;
  ui_settingsButton->setChecked(state);
  if (ui_controlBox) {
    ui_controlBox->setVisible(state);

    if (state)
      ui_settingsButton->setText("X");
    else
      ui_settingsButton->setText("V");
  }
}

void DeviceControl::setMinCurrentValue(double value) {
  ui_currentMinLabel->setText(
      "min: " +
      wlocale.toString(value, DOUBLE_FORMAT, CURRENT_FRACTIAL_SYMBOLS) +
      QString(" ") + CURRENT_UNIT);
  ui_currentSpinBox->setMinimum(value);
}

void DeviceControl::setMaxCurrentValue(double value) {
  ui_currentMaxLabel->setText(
      "max: " +
      wlocale.toString(value, DOUBLE_FORMAT, CURRENT_FRACTIAL_SYMBOLS) +
      QString(" ") + CURRENT_UNIT);
  ui_currentSpinBox->setMaximum(value);
}

void DeviceControl::setMinDurationValue(double value) {
  ui_durationMinLabel->setText(
      "min: " +
      wlocale.toString(value, DOUBLE_FORMAT, DURATION_FRACTIAL_SYMBOLS) +
      QString(" ") + DURATION_UNIT);
  ui_durationSpinBox->setMinimum(value);
}

void DeviceControl::setMaxDurationValue(double value) {
  ui_durationMaxLabel->setText(
      "max: " +
      wlocale.toString(value, DOUBLE_FORMAT, DURATION_FRACTIAL_SYMBOLS) +
      QString(" ") + DURATION_UNIT);
  ui_durationSpinBox->setMaximum(value);
}

void DeviceControl::setMinFrequencyValue(double value) {
  ui_frequencyMinLabel->setText(
      "min: " +
      wlocale.toString(value, DOUBLE_FORMAT, FREQUENCY_FRACTIAL_SYMBOLS) +
      QString(" ") + FREQUENCY_UNIT);
  ui_frequencySpinBox->setMinimum(value);
}

void DeviceControl::setMaxFrequencyValue(double value) {
  ui_frequencyMaxLabel->setText(
      "max: " +
      wlocale.toString(value, DOUBLE_FORMAT, FREQUENCY_FRACTIAL_SYMBOLS) +
      QString(" ") + FREQUENCY_UNIT);
  ui_frequencySpinBox->setMaximum(value);
}

void DeviceControl::setMinVoltageValue(double value) {
  ui_voltageMinLabel->setText(
      "min: " +
      wlocale.toString(value, DOUBLE_FORMAT, VOLTAGE_FRACTIAL_SYMBOLS) +
      QString(" ") + VOLTAGE_UNIT);
  ui_voltageSpinBox->setMinimum(value);
}

void DeviceControl::setMaxVoltageValue(double value) {
  ui_voltageMaxLabel->setText(
      "max: " +
      wlocale.toString(value, DOUBLE_FORMAT, VOLTAGE_FRACTIAL_SYMBOLS) +
      QString(" ") + VOLTAGE_UNIT);
  ui_voltageSpinBox->setMaximum(value);
}

void DeviceControl::setMinDelayValue(double value) {
  ui_delayMinLabel->setText(
      "min: " + wlocale.toString(value, DOUBLE_FORMAT, DELAY_FRACTIAL_SYMBOLS) +
      QString(" ") + DELAY_UNIT);
  ui_delaySpinBox->setMinimum(value);
}

void DeviceControl::setMaxDelayValue(double value) {
  ui_delayMaxLabel->setText(
      "max: " + wlocale.toString(value, DOUBLE_FORMAT, DELAY_FRACTIAL_SYMBOLS) +
      QString(" ") + DELAY_UNIT);
  ui_delaySpinBox->setMaximum(value);
}

void DeviceControl::on_devNameEdit_textEdited(QString newDevName) {
  ui_devNameLabel->setText(newDevName);
  emit setNewDevName(address, newDevName);
}

void DeviceControl::on_currentSpinBox_editingFinished() {
  double value = ui_currentSpinBox->value();
  current = value;
  emit updateParameter(address, CURRENT_SHIFT,
                       qRound(value * pow(10, CURRENT_FRACTIAL_SYMBOLS)));
}

void DeviceControl::on_durationSpinBox_editingFinished() {
  double value = ui_durationSpinBox->value();
  duration = value;
  emit updateParameter(address, DURATION_SHIFT,
                       qRound(value * pow(10, DURATION_FRACTIAL_SYMBOLS)));
}

void DeviceControl::on_frequencySpinBox_editingFinished() {
  double value = ui_frequencySpinBox->value();
  frequency = value;
  emit updateParameter(address, FREQUENCY_SHIFT,
                       qRound(value * pow(10, FREQUENCY_FRACTIAL_SYMBOLS)));
}

void DeviceControl::on_voltageSpinBox_editingFinished() {
  double value = ui_voltageSpinBox->value();
  voltage = value;
  emit updateParameter(address, VOLTAGE_SHIFT,
                       qRound(value * pow(10, VOLTAGE_FRACTIAL_SYMBOLS)));
}

void DeviceControl::on_delaySpinBox_editingFinished() {
  double value = ui_delaySpinBox->value();
  delay = value;
  emit updateParameter(address, DELAY_SHIFT,
                       qRound(value * pow(10, DELAY_FRACTIAL_SYMBOLS)));
}

void DeviceControl::setDeviceState(quint16 value) {
  if (value & POWER_ON_READ_MASK) {
    setPowerButton(true);
  } else {
    setPowerButton(false);
  }

  if (value & START_STOP_READ_MASK) {
    setStartButton(true);
  } else {
    setStartButton(false);
  }

  if (value & MASTER_SLAVE_READ_MASK) {
    setSlaveButton(true);
  } else {
    setSlaveButton(false);
  }
}

void DeviceControl::setStartButton(bool startState) {
  ui_startStopButton->setChecked(startState);
  if (startState) {
    ui_startStopButton->setText(tr("Запущен"));
    ui_startStopButton->setStyleSheet(
        "QPushButton { \
                                            background-color: #00ee00; \
                                            border: 1px solid #00ee00; \
                                            border-radius: 4px; \
                                            color: #fff; \
                                        }");
  } else {
    ui_startStopButton->setText(tr("Остановлен"));
    ui_startStopButton->setStyleSheet(
        "QPushButton { \
                                            background-color: #ee0000; \
                                            border: 1px solid #ee0000; \
                                            border-radius: 4px; \
                                            color: #fff; \
                                        }");
  }
}

void DeviceControl::setSlaveButton(bool state) {
  isSlave = state;
  ui_masterSlaveButton->setChecked(state);
  if (state) {
    ui_masterSlaveButton->setText("Slave");
    ui_masterSlaveButton->setStyleSheet(
        "QPushButton { \
                                            background-color: #00ee00; \
                                            border: 1px solid #00ee00; \
                                            border-radius: 4px; \
                                            color: #fff; \
                                        }");
  } else {
    ui_masterSlaveButton->setText("Master");
    ui_masterSlaveButton->setStyleSheet(
        "QPushButton { \
                                    background-color: #ee0000; \
                                    border: 1px solid #ee0000; \
                                    border-radius: 4px; \
                                    color: #fff; \
                                }");
  }
}

void DeviceControl::on_masterSlaveButton_clicked(bool /*state*/) {
  emit setNewMaster(address, isSlave);
}

void DeviceControl::on_startStopButton_clicked(bool state) {
  if (state) {
    emit updateParameter(address, WRITE_STATUS_SHIFT, START_MASK);
  } else {
    emit updateParameter(address, WRITE_STATUS_SHIFT, STOP_MASK);
  }
}

void DeviceControl::on_powerButton_clicked(bool state) {
  if (state) {
    emit updateParameter(address, WRITE_STATUS_SHIFT, POWER_ON_MASK);
  } else {
    emit updateParameter(address, WRITE_STATUS_SHIFT, POWER_OFF_MASK);
  }
}

void DeviceControl::setPowerButton(bool state) {
  ui_powerButton->setChecked(state);
  if (state) {
    ui_powerButton->setStyleSheet(
        "QPushButton { \
                                      background-color: #00ee00; \
                                      border: 1px solid #00ee00; \
                                      border-radius: 4px; \
                                      color: #fff; \
                                  }");
    ui_powerButton->setText(tr("ВКЛ"));
  } else {
    ui_powerButton->setText(tr("ВЫКЛ"));
    ui_powerButton->setStyleSheet(
        "QPushButton { \
                                      background-color: #ee0000; \
                                      border: 1px solid #ee0000; \
                                      border-radius: 4px; \
                                      color: #fff; \
                                  }");
  }
}
