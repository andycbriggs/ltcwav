#-------------------------------------------------
#
# Project created by QtCreator 2016-09-10T11:59:39
#
#-------------------------------------------------

QT       += core gui

CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LTCWAV
TEMPLATE = app

RC_ICONS = wav.ico

SOURCES += main.cpp\
    libltc/src/decoder.c \
    libltc/src/encoder.c \
    libltc/src/ltc.c \
    libltc/src/timecode.c \
        mainwindow.cpp

HEADERS  += mainwindow.h \
    libltc/src/decoder.h \
    libltc/src/encoder.h \
    libltc/src/ltc.h

FORMS    += mainwindow.ui

OTHER_FILES +=

RESOURCES += \
    UI.qrc
