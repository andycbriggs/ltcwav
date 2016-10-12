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


SOURCES += main.cpp\
        mainwindow.cpp \
    libltc/timecode.c \
    libltc/ltc.c \
    libltc/encoder.c \
    libltc/decoder.c

HEADERS  += mainwindow.h \
    libltc/decoder.h \
    libltc/encoder.h \
    libltc/ltc.h

FORMS    += mainwindow.ui

OTHER_FILES +=

RESOURCES += \
    UI.qrc
