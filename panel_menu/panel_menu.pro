#-------------------------------------------------
#
# Project created by QtCreator 2014-12-26T16:16:15
#
#-------------------------------------------------

QT       += core dbus network

QT       -= gui

TARGET = panel_menu
CONFIG   += console
CONFIG   -= app_bundle

INCLUDEPATH +=./ \
    ./menu/ \
    ./menu/power/ \
    ./menu/network/ \
    ./menu/infomation/


TEMPLATE = app


SOURCES += main.cpp \
    qpanelinterface.cpp \
    qmkey0.cpp \
    qmkey1.cpp \
    qmkey.cpp \
    qmenumain.cpp \
    ./menu/qmenu.cpp \
    ./menu/qdefaultmenu.cpp \
    ./menu/qsettingmenu.cpp \
    menu/power/qrebootmenu.cpp \
    menu/power/qpowermenu.cpp \
    menu/power/qshutdownmenu.cpp \
    menu/network/qnetworkmenu.cpp \
    menu/infomation/qinfomationmenu.cpp \
    menu/infomation/qversionmenu.cpp \
    menu/network/qnetdrivermenu.cpp \
    menu/network/qessidmenu.cpp \
    menu/qinputmenu.cpp \
    menu/network/qreconfignetwork.cpp \
    menu/network/qdhcpmenu.cpp \
    menu/network/qmacmenu.cpp \
    menu/network/qreloadmenu.cpp \
    menu/infomation/qmenuvermenu.cpp \
    menu/power/qsnmenu.cpp \
    menu/infomation/qshowsnmenu.cpp \
    menu/qbackmenu.cpp \
    menu/infomation/qnumbermenu.cpp \
    menu/network/qnet2spimenu.cpp

HEADERS += \
    qpanelinterface.h \
    qmkey0.h \
    qmkey1.h \
    qmkey.h \
    qmenumain.h \
    ./menu/qmenu.h \
    ./menu/qdefaultmenu.h \
    ./menu/qsettingmenu.h \
    menu/power/qrebootmenu.h \
    menu/power/qpowermenu.h \
    menu/power/qshutdownmenu.h \
    menu/network/qnetworkmenu.h \
    menu/infomation/qinfomationmenu.h \
    menu/infomation/qversionmenu.h \
    menu/network/qnetdrivermenu.h \
    menu/network/qessidmenu.h \
    menu/qinputmenu.h \
    menu/network/qreconfignetwork.h \
    menu/network/qdhcpmenu.h \
    menu/network/qmacmenu.h \
    menu/network/qreloadmenu.h \
    menu/infomation/qmenuvermenu.h \
    menu/power/qsnmenu.h \
    menu/infomation/qshowsnmenu.h \
    menu/qbackmenu.h \
    menu/infomation/qnumbermenu.h \
    menu/network/qnet2spimenu.h


FORMS +=

RESOURCES += \
    res/res.qrc
