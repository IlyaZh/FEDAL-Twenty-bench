#-------------------------------------------------
#
# Project created by QtCreator 2018-05-23T15:45:10
#
#-------------------------------------------------

QT       += core gui uitools serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

VERSION_PE_HEADER = 1.1
VERSION = 1.2.0
TARGET = "FEDAL Twenty bench"
TEMPLATE = app

CONFIG += c++17

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
#DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        mainwindow.cpp \
    devicecontrol.cpp \
    models/data_thread.cpp \
    serialporthandler.cpp

HEADERS  += mainwindow.h \
    globals.h \
    devicecontrol.h \
    mocks/serial_mock.hpp \
    models/data_thread.h \
    models/queue.hpp \
    serial_mock.hpp \
    serialporthandler.h \
    crctable.h

FORMS    += \
    devicecontrolform.ui \
    mainwindow.ui

RESOURCES += \
    resourses.qrc

QMAKE_CXXFLAGS += -Wunused-value
QMAKE_CXXFLAGS += -Werror

RC_ICONS = FEDAL.ico
