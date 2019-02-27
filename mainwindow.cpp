#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    serialPort(nullptr)
{
    ui->setupUi(this);

    setupWindow();
}

MainWindow::~MainWindow() {
    if(serialPort != nullptr) {
        serialPort->setOpenState(false);
        delete serialPort;
        serialPort = nullptr;
    }

    delete ui;
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (maybeSave()) {
        writeSettings();
        qInfo() << "Приложение закрыто";
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::setupWindow() {
    ui->firstAddressLabel->setText(QString::number(START_ADDRESS));
    ui->firstAddressSpinBox->setMinimum(START_ADDRESS);
    ui->firstAddressSpinBox->setValue(START_ADDRESS);
    ui->lastAddressSpinBox->setMinimum(START_ADDRESS);
    ui->lastAddressLabel->setText(QString::number(START_ADDRESS+DEVICE_COUNT-1));
    ui->lastAddressSpinBox->setMaximum(START_ADDRESS+DEVICE_COUNT-1);
    ui->firstAddressSpinBox->setMaximum(START_ADDRESS+DEVICE_COUNT-1);
    ui->lastAddressSpinBox->setValue(START_ADDRESS+DEVICE_COUNT-1);



    this->setWindowTitle(APP_TITLE);

    ui->logAddressLabel->setText("Log file dest.: " + LOG_FILE);

    ui->singleControlBox->setVisible(false);
    ui->toggleItemsButton->setVisible(false);

    settings = new QSettings(QSettings::NativeFormat, QSettings::UserScope, ORG_NAME, APP_NAME);
    readSettings();

    currentAddress = START_ADDRESS;

    for (int i = 0; i < 2*DEVICE_COUNT; i++) {
        paramsIsLoaded[i] = false;
    }

    on_refreshPortsButton_clicked(); // refresh com ports
}

// private

void MainWindow::showError(QString msg) {
    qCritical() << "Error: " << msg;
    appendToWLog("Error: " + msg);
    ui->statusBar->showMessage(msg, STATUSBAR_MESSAGE_TIMEOUT);

    if(serialPort != nullptr) {
        serialPort->nextTask();
        if(serialPort->queueIsEmpty())
            requestNextParam();
    }
}

void MainWindow::readSettings() {
    QPoint pos = settings->value("pos", this->pos()).toPoint();
    QSize size = settings->value("size", this->size()).toSize();

    if(pos != QPoint(-1, -1) && size != QSize(-1, -1)) {
        resize(size);
        move(pos);
    }
}

void MainWindow::writeSettings() {
    settings->setValue("pos", pos());
    settings->setValue("size", size());
}

bool MainWindow::maybeSave() {
    QMessageBox::StandardButton ret;
    ret = QMessageBox::warning(this, APP_TITLE,
                               tr("Вы действительно хотите выйти?"),
                               QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes)
        return true;
    else
        return false;
}

void MainWindow::setupDevices(bool state) {
    for (int i = 0; i < 2*DEVICE_COUNT; i++) {
        paramsIsLoaded[i] = false;
    }

    QLayout* vlayout = ui->devicesBox->layout();
        if(state) {
            if(devices.isEmpty()) {
                for(uint8_t i = START_ADDRESS-1; i < DEVICE_COUNT; i++) {
                    DeviceControl *devPtr = new DeviceControl(i+1);
//                    devices.insert(i, devPtr);

                    QString name = settings->value("Dev"+QString::number(i+1), "#"+QString::number(i+1)).toString();
                    devPtr->setDevName(name);

                    devices.append(devPtr);
                    vlayout->addWidget(devPtr->loadWidget());

                    // Обработчик нажатия кнопки "настроить", приводящий к открытию окна изменения параметров устройства
                    connect(devPtr, SIGNAL(showControlBoxSignal(quint8)), this, SLOT(openSettingsWindow(quint8)));
                    // Обработчик события "отправка новых данных"
                    connect(devPtr, SIGNAL(updateParameter(quint8, quint16, quint16)), serialPort, SLOT(dataToWrite(quint8, quint16, quint16)));
                    // Обработчик кнопки установки состояния - ведущий\ведомый
                    connect(devPtr, SIGNAL(setNewMaster(quint8, bool)), this, SLOT(setNewMaster(quint8, bool)));
                    // Обработчки события "изменение имени устройства"
                    connect(devPtr, SIGNAL(setNewDevName(quint8,QString)), this, SLOT(setNewDevName(quint8,QString)));
                }
            }

            currentAddress = START_ADDRESS;
            currentCommandId = 0;
        } else { // state == false
            foreach(DeviceControl* devPtr, devices) {
                vlayout->removeWidget(devPtr->loadWidget());
                devices.removeOne(devPtr);
                devPtr->deleteLater();
            }
        }

}

// slots

void MainWindow::onStateChanged(bool flag) {
    if(flag) {
        ui->portStateLabel->setText(ui->comPortBox->currentText() + " " + ui->comBaudBox->currentText() + " bps");
        ui->connectButton->setText("Отключиться");
        ui->portStateLabel->setStyleSheet("QLabel { \
                                                  background-color: rgb(104, 234, 23); \
                        border: 1px solid rgb(104, 234, 23); \
                border-radius: 4px; \
            }");
            setupDevices(true);
        qInfo() << "Порт открыт: " << ui->comPortBox->currentText() + QString("   ") + ui->comBaudBox->currentText() + " bps";
        appendToWLog("Порт открыт: " + ui->comPortBox->currentText() + QString("   ") + ui->comBaudBox->currentText() + " bps");
        requestNextParam();
    } else {
        ui->portStateLabel->setText("НЕ подключен");
        ui->portStateLabel->setStyleSheet("QLabel { \
                                          background-color: rgb(240, 0, 0); \
                                        border: 1px solid rgb(240, 0, 0); \
                                        border-radius: 4px; \
                                        }");
        ui->connectButton->setText("Подключиться");
        setupDevices(false);
        qInfo() << "Порт закрыт";
        appendToWLog("Порт закрыт");
    }
}

void MainWindow::readReady() {


    quint16 startRegister = serialPort->getRegister();
    qint8 addr = serialPort->getAddress();
    if(startRegister >= readRegisters[SerialPortHandler::MIN_REGS_SHIFT] && startRegister < readRegisters[SerialPortHandler::MIN_REGS_SHIFT]+countRegisters[SerialPortHandler::MIN_REGS_SHIFT])
        paramsIsLoaded[2*(addr-1)] = true;
    else if (startRegister >= readRegisters[SerialPortHandler::MAX_REGS_SHIFT] && startRegister < readRegisters[SerialPortHandler::MAX_REGS_SHIFT]+countRegisters[SerialPortHandler::MAX_REGS_SHIFT])
        paramsIsLoaded[2*(addr-1)+1] = true;

    for(quint8 i = 0; i < serialPort->getCount(); i++) {
        quint16 item = serialPort->getValues().at(i);
        setDevParam(addr, startRegister+i, item);
    }

    serialPort->nextTask();
    if(serialPort->queueIsEmpty())
        requestNextParam();
}


void MainWindow::setDevParam(quint8 address, quint16 reg, quint16 value) {

    if (address > devices.size() || address == 0) return;

    DeviceControl *devPtr = devices.at(address-1);

    devPtr->setLink(DeviceControl::LINK_STATE);

    switch (reg) {
    case DeviceControl::FREQUENCY_SHIFT:
        devPtr->setFrequencyValue(value/pow(10, FREQUENCY_FRACTIAL_SYMBOLS));
        break;
    case DeviceControl::DURATION_SHIFT:
        devPtr->setDurationValue(value);
        break;
    case DeviceControl::CURRENT_SHIFT:
        devPtr->setCurrentValue(value/pow(10, CURRENT_FRACTIAL_SYMBOLS));
        break;
    case DeviceControl::VOLTAGE_SHIFT:
        devPtr->setVoltageValue(value/pow(10, VOLTAGE_FRACTIAL_SYMBOLS));
        break;
    case DeviceControl::DELAY_SHIFT:
        devPtr->setDelayValue(value);
        break;
    case DeviceControl::MIN_FREQUENCY_SHIFT:
        devPtr->setMinFrequencyValue(value/pow(10, FREQUENCY_FRACTIAL_SYMBOLS));
        break;
    case DeviceControl::MAX_FREQUENCY_SHIFT:
        devPtr->setMaxFrequencyValue(value/pow(10, FREQUENCY_FRACTIAL_SYMBOLS));
        break;
    case DeviceControl::MIN_DURATION_SHIFT:
        devPtr->setMinDurationValue(value);
        break;
    case DeviceControl::MAX_DURATION_SHIFT:
        devPtr->setMaxDurationValue(value);
        break;
    case DeviceControl::MIN_CURRENT_SHIFT:
        devPtr->setMinCurrentValue(value/pow(10, CURRENT_FRACTIAL_SYMBOLS));
        break;
    case DeviceControl::MAX_CURRENT_SHIFT:
        devPtr->setMaxCurrentValue(value/pow(10, CURRENT_FRACTIAL_SYMBOLS));
        break;
    case DeviceControl::MIN_VOLTAGE_SHIFT:
        devPtr->setMinVoltageValue(value/pow(10, VOLTAGE_FRACTIAL_SYMBOLS));
        break;
    case DeviceControl::MAX_VOLTAGE_SHIFT:
        devPtr->setMaxVoltageValue(value/pow(10, VOLTAGE_FRACTIAL_SYMBOLS));
        break;
    case DeviceControl::MIN_DELAY_SHIFT:
        devPtr->setMinDelayValue(value);
        break;
    case DeviceControl::MAX_DELAY_SHIFT:
        devPtr->setMaxDelayValue(value);
        break;
    case DeviceControl::STATUS_READ_SHIFT:
        devPtr->setDeviceState(value);
        break;
    default:
        break;
    }
}

void MainWindow::onTimeout(quint8 devAddress) {
    if(devAddress <= devices.size()) {
        devices.at(devAddress-1)->setLink(DeviceControl::NO_LINK_STATE);
    }

    QString error = QString::number(devAddress) + QString(" ") + serialPort->errorString();
    showError(error);
}

void MainWindow::requestNextParam() {
    if(!serialPort->isOpen()) return;

    currentCommandId = SerialPortHandler::VALUES_REGS_SHIFT;
    if(paramsIsLoaded[2*(currentAddress-1)+1] == false) {
        currentCommandId = SerialPortHandler::MAX_REGS_SHIFT;
    }

    if(paramsIsLoaded[2*(currentAddress-1)] == false) {
        currentCommandId = SerialPortHandler::MIN_REGS_SHIFT;
    }

    serialPort->dataToRead(currentAddress, readRegisters[currentCommandId], countRegisters[currentCommandId]);

    currentAddress +=1 ;
    if(currentAddress > ui->lastAddressLabel->text().toInt()) {
        currentAddress = ui->firstAddressLabel->text().toInt();
    }
//    if(currentAddress > DEVICE_COUNT) {
//        currentAddress = START_ADDRESS;
//    }
}

void MainWindow::on_refreshPortsButton_clicked()
{
    ui->comBaudBox->clear();
    ui->comPortBox->clear();

    QString comPort = settings->value("comPort", "").toString();
    QString comBaudRate = settings->value("comBaudRate", 9600).toString();


    foreach(QSerialPortInfo spi, QSerialPortInfo::availablePorts()) {
        ui->comPortBox->addItem(spi.portName());
        if(spi.portName() == comPort) ui->comPortBox->setCurrentText(comPort);
    }

    foreach(quint32 br, BAUDRATES) {
        QString baud = QString::number(br);
        ui->comBaudBox->addItem(baud);
        if(baud == comBaudRate) ui->comBaudBox->setCurrentText(baud);
    }
}


void MainWindow::on_connectButton_clicked()
{
    QString port = ui->comPortBox->currentText();
    if(port.isEmpty()) return;

    settings->setValue("comPort", ui->comPortBox->currentText());
    settings->setValue("comBaudRate", ui->comBaudBox->currentText().toInt());

    if(serialPort == nullptr)
        serialPort = new SerialPortHandler(ui->comPortBox->currentText(), ui->comBaudBox->currentText().toInt(), COM_TIMEOUT, this);

    if(serialPort->isOpen()) {
        serialPort->setOpenState(false);
        ui->connectButton->setChecked(false);
        disconnect(serialPort, 0, 0, 0);
    } else {
//        currentAddress = START_ADDRESS;
        currentAddress = ui->firstAddressLabel->text().toInt();
        currentCommandId = 0;
        serialPort->setTimeout(COM_TIMEOUT);
        connect(serialPort, SIGNAL(stateChanged(bool)), this, SLOT(onStateChanged(bool)));
        connect(serialPort, SIGNAL(appendToLog(QString)), this, SLOT(appendToWLog(QString)));

        connect(serialPort, &SerialPortHandler::errorOccuredSignal, [this]() {
            showError(serialPort->errorString());
        });
        connect(serialPort, SIGNAL(timeoutSignal(quint8)), this, SLOT(onTimeout(quint8)));
        //connect(serialPort, SIGNAL(stateChanged(bool)), this, SLOT(onStateChanged(bool)));
        connect(serialPort, SIGNAL(newDataIsReady()), this, SLOT(readReady()));


        serialPort->setBaud(ui->comBaudBox->currentText().toInt());
        serialPort->setPort(ui->comPortBox->currentText());
        serialPort->setTimeout(COM_TIMEOUT);
        serialPort->setOpenState(true);
        if(!serialPort->isOpen()) {
            showError("Connection failed");
            ui->connectButton->setChecked(false);
        }
    }
}

void MainWindow::openSettingsWindow(quint8 address) {
    foreach(DeviceControl *item, devices) {

        if((*item).getAddress() != address) {
            (*item).showControl(false);
        }
    }

}

void MainWindow::setNewDevName(quint8 address, QString name) {
    settings->setValue("Dev"+QString::number(address), name);
}

void MainWindow::setNewMaster(quint8 address, bool isSlave) {
    foreach(DeviceControl *item, devices) {
        if((*item).getAddress() != address && isSlave == false) {
            serialPort->dataToWrite((*item).getAddress(), DeviceControl::WRITE_STATUS_SHIFT, DeviceControl::SLAVE_MASK);
        }
    }

    if (isSlave) // Текущее состояние
        serialPort->dataToWrite(address, DeviceControl::WRITE_STATUS_SHIFT, DeviceControl::MASTER_MASK);
    else
        serialPort->dataToWrite(address, DeviceControl::WRITE_STATUS_SHIFT, DeviceControl::SLAVE_MASK);
}

//void MainWindow::prepareDataToWrite(quint8 address, quint16 startRegister, quint16 value) {
//        serialPort->dataToWrite(address, startRegister, value);
//}


void MainWindow::on_delaySpinBox_editingFinished()
{
    if(serialPort == nullptr) return;

    int arg1 = ui->delaySpinBox->value();
    serialPort->setTimeout(arg1);
    ui->delayLabel->setText(QString::number(arg1) + " мс");
}

void MainWindow::on_lastAddressSpinBox_editingFinished()
{
    int arg1 = ui->lastAddressSpinBox->value();
    ui->lastAddressLabel->setText(QString::number(arg1));
    ui->firstAddressSpinBox->setMaximum(arg1);
}

void MainWindow::on_firstAddressSpinBox_editingFinished()
{
    int arg1 = ui->firstAddressSpinBox->value();
    ui->firstAddressLabel->setText(QString::number(arg1));
    ui->lastAddressSpinBox->setMinimum(arg1);
}

void MainWindow::appendToWLog(QString msg) {
    ui->log->appendPlainText("[" + QDateTime::currentDateTime().toString("dd.MM.yy hh:mm:ss:zzz") + "] " + msg + QString('\r'));
}
