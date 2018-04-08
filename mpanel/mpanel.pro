#-------------------------------------------------
#
# Project created by QtCreator 2014-11-21T14:11:38
#
#-------------------------------------------------

QT       += core dbus

QT       -= gui

TARGET = mpanel
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    qpaneladaptor.cpp \
    qpaneldbus.cpp \
    qpanel.cpp \
    posix_qextserialport.cpp \
    qextserialbase.cpp \
    qprotocol.cpp

HEADERS += \
    qpaneldbus.h \
    qpaneladaptor.h \
    qpanel.h \
    posix_qextserialport.h \
    qextserialbase.h \
    qprotocol.h
