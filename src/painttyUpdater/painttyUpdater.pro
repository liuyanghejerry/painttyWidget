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

mac {
    # Prevent qmlplugindump from popping up in the dock when launched.
    # We embed the Info.plist file, so the application doesn't need to
    # be a bundle.
    QMAKE_LFLAGS += -sectcreate __TEXT __info_plist $$shell_quote($$PWD/Info.plist)
    #we still keep app_bundle, because we need to call macdeployqt
    #CONFIG -= app_bundle
}

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

OTHER_FILES += \
    Info.plist

