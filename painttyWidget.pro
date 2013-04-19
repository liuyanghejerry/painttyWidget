#-------------------------------------------------
#
# Project created by QtCreator 2012-05-26T18:10:10
#
#-------------------------------------------------

DEFINES += PAINTTY_DEV
QT       += core gui network widgets

win32 {
     RC_FILE = app.rc
}

mac {
    macx-clang: warning("if you encounter \"fatal error: \'initializer_list\' file not found\", try using makespecs \"macx-clang-libc++\"")
    ICON = iconset/icon.icns
}

CONFIG += c++11

TARGET = MrPaint
TEMPLATE = app


SOURCES += main.cpp\
    widgets/mainwindow.cpp \
    widgets/canvas.cpp \
    misc/layermanager.cpp \
    network/messagesocket.cpp \
    widgets/colorwheel.cpp \
    widgets/roomlistdialog.cpp \
    network/socket.cpp \
    widgets/colorgriditem.cpp \
    widgets/colorgrid.cpp \
    widgets/flowlayout.cpp \
    widgets/layerwidgetheader.cpp \
    widgets/layerwidget.cpp \
    widgets/layerlabel.cpp \
    widgets/layeritem.cpp \
    widgets/iconcheckbox.cpp \
    misc/layer.cpp \
    paintingTools/brush/sketchbrush.cpp \
    paintingTools/brush/eraser.cpp \
    paintingTools/brush/brush.cpp \
    paintingTools/brush/pencil.cpp \
    widgets/colorspinboxgroup.cpp \
    widgets/colorbox.cpp \
    widgets/newroomwindow.cpp \
    network/commandsocket.cpp \
    widgets/aboutdialog.cpp \
    misc/singleshortcut.cpp \
    widgets/canvascontainer.cpp \
    widgets/memberlistwidget.cpp \
    misc/errortable.cpp \
    paintingTools/brush/abstractbrush.cpp \
    paintingTools/brush/brushmanager.cpp \
    paintingTools/brush/wetfingure.cpp \
    widgets/brushsettingswidget.cpp \
    widgets/helpdialog.cpp \
    widgets/panoramaview.cpp \
    widgets/panoramawidget.cpp \
    widgets/panoramaslider.cpp \
    network/localnetworkinterface.cpp

HEADERS  += widgets/mainwindow.h \
    widgets/canvas.h \
    misc/layermanager.h \
    network/messagesocket.h \
    network/datasocket.h \
    widgets/colorwheel.h \
    widgets/roomlistdialog.h \
    network/socket.h \
    widgets/colorgriditem.h \
    widgets/colorgrid.h \
    widgets/flowlayout.h \
    widgets/layerwidgetheader.h \
    widgets/layerwidget.h \
    widgets/layerlabel.h \
    widgets/layeritem.h \
    widgets/iconcheckbox.h \
    misc/layer.h \
    paintingTools/brush/sketchbrush.h \
    paintingTools/brush/eraser.h \
    paintingTools/brush/brush.h \
    paintingTools/brush/pencil.h \
    widgets/colorspinboxgroup.h \
    widgets/colorbox.h \
    widgets/newroomwindow.h \
    network/commandsocket.h \
    widgets/aboutdialog.h \
    common.h \
    misc/singleshortcut.h \
    widgets/canvascontainer.h \
    widgets/memberlistwidget.h \
    misc/router.h \
    misc/errortable.h \
    paintingTools/brush/abstractbrush.h \
    paintingTools/brush/brushmanager.h \
    paintingTools/brush/wetfingure.h \
    widgets/brushsettingswidget.h \
    widgets/helpdialog.h \
    widgets/panoramaview.h \
    widgets/panoramawidget.h \
    widgets/panoramaslider.h \
    network/localnetworkinterface.h
FORMS    += widgets/mainwindow.ui \
    widgets/roomlistdialog.ui \
    widgets/colorspinboxgroup.ui \
    widgets/colorbox.ui \
    widgets/newroomwindow.ui \
    widgets/aboutdialog.ui \
    widgets/helpdialog.ui

UI_DIR = $${PWD}/widgets/

TRANSLATIONS += translation/paintty_zh_CN.ts \ #Simplified Chinese
    translation/paintty_zh_TW.ts \ #Traditional Chinese
#    translation/paintty_zh_HK.ts \
#    translation/paintty_zh_MO.ts
    translation/paintty_ja.ts #Japanese

RESOURCES += resources.qrc
