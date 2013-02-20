#include <QApplication>
#include <QTranslator>
#include "widgets/mainwindow.h"
#include "widgets/roomlistdialog.h"

#ifdef QT_PLUGIN_STATIC_LINK
#include<QtCore/QtPlugin>
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
//Q_IMPORT_PLUGIN(QICOPlugin)
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setStyle("Fusion");
    auto p = QApplication::palette();
    //    p.setColor(QPalette::Window, QColor::fromRgb(54, 86, 60));
    //    p.setColor(QPalette::WindowText, QColor::fromRgb(168, 216, 185));
    //    p.setColor(QPalette::Base, QColor::fromRgb(34, 125, 81));
    //    p.setColor(QPalette::AlternateBase, QColor::fromRgb(27, 129, 62));
    //    p.setColor(QPalette::ToolTipBase, QColor::fromRgb(27, 129, 62));
    //    p.setColor(QPalette::ToolTipText, QColor::fromRgb(54, 86, 60));
    p.setColor(QPalette::Text, QColor::fromRgb(54, 86, 60));
    p.setColor(QPalette::Button, QColor::fromRgb(168, 216, 185));
    p.setColor(QPalette::ButtonText, QColor::fromRgb(0, 0, 0));
    p.setColor(QPalette::BrightText, QColor::fromRgb(168, 216, 185));
    p.setColor(QPalette::Highlight, QColor::fromRgb(168, 216, 185));
    p.setColor(QPalette::HighlightedText, QColor::fromRgb(54, 86, 60));
    QApplication::setPalette(p);
    QTranslator qtTranslator;
    qtTranslator.load("translation/qt_" + QLocale::system().name());
    a.installTranslator(&qtTranslator);

    QTranslator myappTranslator;
    myappTranslator.load("translation/paintty_" + QLocale::system().name());
    a.installTranslator(&myappTranslator);

    RoomListDialog *dialog = new RoomListDialog;
    while(dialog->exec()) {
        dialog->hide();
        MainWindow w;
        w.init();
        w.setNickName(dialog->nick());
        w.setRoomName(dialog->roomName());
        w.setHistorySize(dialog->historySize());
        w.socketInit(dialog->dataPort(),dialog->msgPort());
        w.show();
        w.showMaximized();
        a.exec();
    }

    delete dialog;
    a.quit();

    return 0;
}
