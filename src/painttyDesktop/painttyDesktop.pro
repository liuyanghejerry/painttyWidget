#-------------------------------------------------
#
# Project created by QtCreator 2012-05-26T18:10:10
#
#-------------------------------------------------

DEFINES += PAINTTY_DEV
QT       += core gui network widgets script

DEFINES += PAINTTY_DESKTOP

DEFINES += PAINTTY_NO_TABLET

win32 {
    RC_FILE = app.rc
    LIBS += -limm32
    SOURCES +=
    HEADERS +=
}

!win32 {
    SOURCES +=
    HEADERS +=
}

mac {
    macx-clang: warning("if you encounter \"fatal error: \'initializer_list\' file not found\", try using makespecs \"macx-clang-libc++\"")
    ICON = iconset/icon.icns
}

include(../../commonconfigure.pri)

CONFIG += c++11

TARGET = MrPaint
TEMPLATE = app

SOURCES += main.cpp\
    widgets/mainwindow.cpp \
    widgets/canvas.cpp \
    misc/layermanager.cpp \
    widgets/colorwheel.cpp \
    widgets/roomlistdialog.cpp \
    ../common/network/socket.cpp \
    widgets/colorgriditem.cpp \
    widgets/colorgrid.cpp \
    widgets/flowlayout.cpp \
    widgets/layerwidgetheader.cpp \
    widgets/layerwidget.cpp \
    widgets/layerlabel.cpp \
    widgets/layeritem.cpp \
    widgets/iconcheckbox.cpp \
    misc/layer.cpp \
    widgets/colorspinboxgroup.cpp \
    widgets/colorbox.cpp \
    widgets/newroomwindow.cpp \
    widgets/aboutdialog.cpp \
    misc/singleshortcut.cpp \
    widgets/canvascontainer.cpp \
    widgets/memberlistwidget.cpp \
    misc/errortable.cpp \
    paintingTools/brush/brushmanager.cpp \
    widgets/brushsettingswidget.cpp \
    widgets/helpdialog.cpp \
    widgets/panoramaview.cpp \
    widgets/panoramawidget.cpp \
    widgets/panoramaslider.cpp \
    ../common/network/localnetworkinterface.cpp \
    misc/platformextend_win32.cpp \
    misc/platformextend_dummy.cpp \
    misc/platformextend.cpp \
    misc/shortcutmanager.cpp \
    widgets/configuredialog.cpp\
    ../common/network/clientsocket.cpp \
    misc/archivefile.cpp \
    ../common/network/packparser.cpp \
    widgets/clearlineedit.cpp \
    widgets/roomsharebar.cpp \
    widgets/easycopylineedit.cpp \
    widgets/gradualbox.cpp \
    widgets/developerconsole.cpp \
    widgets/canvasbackend.cpp \
    widgets/irclineedit.cpp \
    widgets/waitupdaterdialog.cpp \
    paintingTools/brush/abstractbrush.cpp \
    paintingTools/brush/basicbrush.cpp \
    paintingTools/brush/basiceraser.cpp \
    paintingTools/brush/binarybrush.cpp \
    paintingTools/brush/brushfeature.cpp \
    paintingTools/brush/maskbased.cpp \
    paintingTools/brush/sketchbrush.cpp \
    paintingTools/brush/waterbased.cpp \
    widgets/panoramarotator.cpp \
    widgets/networkindicator.cpp


HEADERS  += widgets/mainwindow.h \
    widgets/canvas.h \
    misc/layermanager.h \
    widgets/colorwheel.h \
    widgets/roomlistdialog.h \
    ../common/network/socket.h \
    widgets/colorgriditem.h \
    widgets/colorgrid.h \
    widgets/flowlayout.h \
    widgets/layerwidgetheader.h \
    widgets/layerwidget.h \
    widgets/layerlabel.h \
    widgets/layeritem.h \
    widgets/iconcheckbox.h \
    misc/layer.h \
    widgets/colorspinboxgroup.h \
    widgets/colorbox.h \
    widgets/newroomwindow.h \
    widgets/aboutdialog.h \
    ../common/common.h \
    misc/singleshortcut.h \
    widgets/canvascontainer.h \
    widgets/memberlistwidget.h \
    misc/router.h \
    misc/errortable.h \
    widgets/brushsettingswidget.h \
    widgets/helpdialog.h \
    widgets/panoramaview.h \
    widgets/panoramawidget.h \
    widgets/panoramaslider.h \
    ../common/network/localnetworkinterface.h \
    misc/platformextend.h \
    misc/singleton.h \
    misc/call_once.h \
    misc/shortcutmanager.h \
    widgets/configuredialog.h\
    ../common/network/clientsocket.h \
    misc/archivefile.h \
    ../common/network/packparser.h \
    widgets/clearlineedit.h \
    widgets/roomsharebar.h \
    widgets/easycopylineedit.h \
    widgets/gradualbox.h \
    widgets/developerconsole.h \
    widgets/irclineedit.h \
    widgets/canvasbackend.h \
    widgets/waitupdaterdialog.h \
    paintingTools/brush/brushmanager.h \
    paintingTools/brush/abstractbrush.h \
    paintingTools/brush/basicbrush.h \
    paintingTools/brush/basiceraser.h \
    paintingTools/brush/binarybrush.h \
    paintingTools/brush/brushfeature.h \
    paintingTools/brush/brushsettings.h \
    paintingTools/brush/maskbased.h \
    paintingTools/brush/sketchbrush.h \
    paintingTools/brush/waterbased.h \
    widgets/panoramarotator.h \
    widgets/networkindicator.h

FORMS    += widgets/mainwindow.ui \
    widgets/roomlistdialog.ui \
    widgets/colorspinboxgroup.ui \
    widgets/colorbox.ui \
    widgets/newroomwindow.ui \
    widgets/aboutdialog.ui \
    widgets/helpdialog.ui \
    widgets/configuredialog.ui \
    widgets/developerconsole.ui \
    widgets/waitupdaterdialog.ui

TRANSLATIONS += translation/paintty_zh_CN.ts \ #Simplified Chinese
    translation/paintty_zh_TW.ts \ #Traditional Chinese
#    translation/paintty_zh_HK.ts \
#    translation/paintty_zh_MO.ts
    translation/paintty_ja.ts #Japanese

RESOURCES += resources.qrc
