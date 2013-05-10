#-------------------------------------------------
#
# Project created by QtCreator 2013-04-29T16:45:49
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = updater
TEMPLATE = app

CONFIG += c++11


SOURCES += main.cpp\
    ../common/network/socket.cpp \
    updater.cpp \
    ../common/network/localnetworkinterface.cpp

HEADERS  += \
    ../common/common.h \
    ../common/network/socket.h \
    updater.h \
    ../common/network/localnetworkinterface.h

TRANSLATIONS += translation/updater_zh_CN.ts \ #Simplified Chinese
    translation/updater_zh_TW.ts \ #Traditional Chinese
#    translation/updater_zh_HK.ts \
#    translation/updater_zh_MO.ts
    translation/updater_ja.ts #Japanese

RESOURCES += \
    resources.qrc

