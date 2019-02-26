#ifndef DEVICECONTROL_H
#define DEVICECONTROL_H

#include <QWidget>
#include <QUiLoader>
#include <QFile>
#include <QGroupBox>
#include <QPointer>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QRadioButton>
#include <QDebug>
#include <QLineEdit>

#include "globals.h"

class DeviceControl : public QWidget
{
    Q_OBJECT
public:
    explicit DeviceControl(quint8 address, QWidget *parent = nullptr);
    ~DeviceControl();
    QString getDevName();
    quint8 getAddress();
    double getCurrentValue();
    double getFrequencyValue();
    double getDurationValue();
    double getVoltageValue();
    double getDelayValue();
    bool getLink();
    void setCurrentValue(double);
    void setDurationValue(double);
    void setFrequencyValue(double);
    void setVoltageValue(double);
    void setDelayValue(double);
    void setDevName(QString);
    void setMinCurrentValue(double);
    void setMaxCurrentValue(double);
    void setMinDurationValue(double);
    void setMaxDurationValue(double);
    void setMinFrequencyValue(double);
    void setMaxFrequencyValue(double);
    void setMinVoltageValue(double);
    void setMaxVoltageValue(double);
    void setMinDelayValue(double);
    void setMaxDelayValue(double);
    typedef enum {LINK_STATE = 1, NO_LINK_STATE = 2, ERROR_STATE = 3} linkState_t;
    void setLink(linkState_t);
    void showControl(bool);
    void setDeviceState(quint16);
    void setStartButton(bool);
    void setSlaveButton(bool);
    void setPowerButton(bool);
    QWidget* loadWidget();
    enum {FREQUENCY_SHIFT = 0x0001,
               MIN_FREQUENCY_SHIFT = 0x0011,
               MAX_FREQUENCY_SHIFT = 0x0021,
               DURATION_SHIFT = 0x0002,
               MIN_DURATION_SHIFT = 0x0012,
               MAX_DURATION_SHIFT = 0x0022,
               CURRENT_SHIFT = 0x0003,
               MIN_CURRENT_SHIFT = 0x0013,
               MAX_CURRENT_SHIFT = 0x0023,
               VOLTAGE_SHIFT = 0x0004,
               MIN_VOLTAGE_SHIFT = 0x0014,
               MAX_VOLTAGE_SHIFT = 0x0024,
               DELAY_SHIFT = 0x0005,
               MIN_DELAY_SHIFT = 0x0015,
               MAX_DELAY_SHIFT = 0x0025,
               STATUS_READ_SHIFT = 0x0007,
               BLOCKS_SHIFT = 0x0008,
               WRITE_STATUS_SHIFT = 0x0009} commandsShift;
       enum {POWER_ON_MASK = 0x02, POWER_OFF_MASK = 0x04, START_MASK = 0x08, STOP_MASK = 0x10, MASTER_MASK = 0x20, SLAVE_MASK = 0x40, EXT_SYNC_MASK = 1<<5, INT_SYNC_MASK = 1<<6} statusWriteMask;
       enum {POWER_ON_READ_MASK = 1, START_STOP_READ_MASK = 1<<1, MASTER_SLAVE_READ_MASK = 1<<2, INT_SYNC_READ_MASK = 1<<2} statusReadMask;
       enum {EXT_BLOCK = 1<<1, TEMP_BLOCK = 1<<4} blocksReadMask;


private:
    QWidget* mainWidget;
    QPointer<QLabel> ui_devNameLabel;
    QPointer<QLabel> ui_currentValueLabel;
    QPointer<QLabel> ui_durationValueLabel;
    QPointer<QLabel> ui_frequencyValueLabel;
    QPointer<QLabel> ui_voltageValueLabel;
    QPointer<QLabel> ui_delayValueLabel;
    QPointer<QLabel> ui_linkLabel;
    QPointer<QLabel> ui_currentMaxLabel;
    QPointer<QLabel> ui_durationMaxLabel;
    QPointer<QLabel> ui_frequencyMaxLabel;
    QPointer<QLabel> ui_voltageMaxLabel;
    QPointer<QLabel> ui_delayMaxLabel;
    QPointer<QLabel> ui_currentMinLabel;
    QPointer<QLabel> ui_durationMinLabel;
    QPointer<QLabel> ui_frequencyMinLabel;
    QPointer<QLabel> ui_voltageMinLabel;
    QPointer<QLabel> ui_delayMinLabel;
    QPointer<QPushButton> ui_startStopButton;
    QPointer<QPushButton> ui_powerButton;
    QPointer<QPushButton> ui_masterSlaveButton;
    QPointer<QPushButton> ui_settingsButton;
    QPointer<QLineEdit> ui_devNameEdit;
    QPointer<QDoubleSpinBox> ui_currentSpinBox;
    QPointer<QDoubleSpinBox> ui_durationSpinBox;
    QPointer<QDoubleSpinBox> ui_frequencySpinBox;
    QPointer<QDoubleSpinBox> ui_voltageSpinBox;
    QPointer<QDoubleSpinBox> ui_delaySpinBox;
    QPointer<QGroupBox> ui_controlBox;
    quint8 address;
    double current, duration, frequency, voltage, delay;
    bool link;
    bool controlIsShown;
    bool isSlave;

signals:
    void showControlBoxSignal(quint8);
    void updateParameter(quint8, quint16, quint16);
    void setNewMaster(quint8, bool);
    void setNewDevName(quint8, QString);

public slots:


private slots:
    void on_settingsButton_clicked(bool);
    void on_devNameEdit_textEdited(QString);
    void on_currentSpinBox_editingFinished();
    void on_durationSpinBox_editingFinished();
    void on_frequencySpinBox_editingFinished();
    void on_voltageSpinBox_editingFinished();
    void on_delaySpinBox_editingFinished();
    void on_startStopButton_clicked(bool);
    void on_masterSlaveButton_clicked(bool);
    void on_powerButton_clicked(bool);
};

#endif // DEVICECONTROL_H
