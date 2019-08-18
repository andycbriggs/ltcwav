#-------------------------------------------------
#
# Project created by QtCreator 2016-09-10T11:59:39
#
#-------------------------------------------------

QT       += core gui widgets

CONFIG += c++11

TARGET = LTCWAV
TEMPLATE = app

VERSION = 1

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
