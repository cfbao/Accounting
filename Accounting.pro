#-------------------------------------------------
#
# Project created by QtCreator 2016-09-12T10:34:16
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ATAc
TEMPLATE = app
CONFIG += c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    commands.cpp \
    dateedit.cpp \
    ledger.cpp \
    ledgerdelegate.cpp \
    ledgeritem.cpp \
    ledgervheader.cpp \
    ledgerview.cpp \
    myfileinfo.cpp

HEADERS  += mainwindow.h \
    commands.h \
    dateedit.h \
    ledger.h \
    ledgerdelegate.h \
    ledgeritem.h \
    ledgervheader.h \
    ledgerview.h \
    myfileinfo.h

FORMS    += mainwindow.ui

DISTFILES += \
    .gitignore
