#-------------------------------------------------
#
# Project created by QtCreator 2012-05-26T18:10:10
#
#-------------------------------------------------


DEFINES += DEBUG

QT       += core gui network widgets

win32 {
     RC_FILE = app.rc
}

CONFIG += c++11

TARGET = painttyWidget
TEMPLATE = app


SOURCES += main.cpp\
        widgets/mainwindow.cpp \
    widgets/canvas.cpp \
    layermanager.cpp \
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
    layer.cpp \
    widgets/developerconsole.cpp \
    paintingTools/brush/sketchbrush.cpp \
    paintingTools/brush/eraser.cpp \
    paintingTools/brush/brush.cpp \
    paintingTools/brush/pencil.cpp \
    widgets/colorspinboxgroup.cpp \
    paintingTools/colorpicker.cpp \
    widgets/colorbox.cpp \
    widgets/newroomwindow.cpp \
    network/commandsocket.cpp \
    widgets/aboutdialog.cpp

HEADERS  += widgets/mainwindow.h \
    widgets/canvas.h \
    layermanager.h \
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
    layer.h \
    widgets/developerconsole.h \
    paintingTools/brush/sketchbrush.h \
    paintingTools/brush/eraser.h \
    paintingTools/brush/brush.h \
    paintingTools/brush/pencil.h \
    widgets/colorspinboxgroup.h \
    paintingTools/colorpicker.h \
    widgets/colorbox.h \
    widgets/newroomwindow.h \
    network/commandsocket.h \
    widgets/aboutdialog.h

FORMS    += widgets/mainwindow.ui \
    widgets/roomlistdialog.ui \
    widgets/developerconsole.ui \
    widgets/colorspinboxgroup.ui \
    widgets/colorbox.ui \
    widgets/newroomwindow.ui \
    widgets/aboutdialog.ui
UI_DIR = widgets/

TRANSLATIONS += translations/paintty_zh_CN.ts

RESOURCES += resources.qrc
