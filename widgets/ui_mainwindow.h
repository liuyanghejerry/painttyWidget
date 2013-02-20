/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.0.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "canvas.h"
#include "colorbox.h"
#include "colorgrid.h"
#include "layerwidget.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionExport_All;
    QAction *actionExport_Visiable;
    QAction *action_Quit;
    QAction *action_About_Mr_Paint;
    QAction *actionAbout_Qt;
    QAction *actionExport_Visible_To_ClipBorad;
    QAction *actionExport_All_To_Clipboard;
    QAction *actionReset_View;
    QAction *actionNothing_Here;
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout_6;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;
    QVBoxLayout *verticalLayout_5;
    Canvas *canvas;
    QSpacerItem *verticalSpacer;
    QDockWidget *dockWidget;
    QWidget *dockWidgetContents;
    QVBoxLayout *verticalLayout_4;
    ColorBox *colorBox;
    ColorGrid *colorGrid;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label;
    QSpinBox *spinBox;
    QVBoxLayout *verticalLayout_3;
    QPushButton *pencilButton;
    QPushButton *brushButton;
    QPushButton *sketchButton;
    QPushButton *eraserButton;
    QPushButton *colorPicker;
    QSpacerItem *verticalSpacer_2;
    QDockWidget *dockWidget_2;
    QWidget *dockWidgetContents_2;
    QVBoxLayout *verticalLayout;
    QTextEdit *textEdit;
    QHBoxLayout *horizontalLayout;
    QLineEdit *lineEdit;
    QPushButton *pushButton;
    QDockWidget *dockWidget_3;
    QWidget *dockWidgetContents_3;
    QVBoxLayout *verticalLayout_2;
    LayerWidget *layerWidget;
    QMenuBar *menuBar;
    QMenu *menu_File;
    QMenu *menuExport;
    QMenu *menu_Edit;
    QMenu *menu_About;
    QMenu *menu_View;
    QButtonGroup *buttonGroup;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(982, 684);
        actionExport_All = new QAction(MainWindow);
        actionExport_All->setObjectName(QStringLiteral("actionExport_All"));
        actionExport_Visiable = new QAction(MainWindow);
        actionExport_Visiable->setObjectName(QStringLiteral("actionExport_Visiable"));
        action_Quit = new QAction(MainWindow);
        action_Quit->setObjectName(QStringLiteral("action_Quit"));
        action_About_Mr_Paint = new QAction(MainWindow);
        action_About_Mr_Paint->setObjectName(QStringLiteral("action_About_Mr_Paint"));
        actionAbout_Qt = new QAction(MainWindow);
        actionAbout_Qt->setObjectName(QStringLiteral("actionAbout_Qt"));
        actionExport_Visible_To_ClipBorad = new QAction(MainWindow);
        actionExport_Visible_To_ClipBorad->setObjectName(QStringLiteral("actionExport_Visible_To_ClipBorad"));
        actionExport_All_To_Clipboard = new QAction(MainWindow);
        actionExport_All_To_Clipboard->setObjectName(QStringLiteral("actionExport_All_To_Clipboard"));
        actionReset_View = new QAction(MainWindow);
        actionReset_View->setObjectName(QStringLiteral("actionReset_View"));
        actionNothing_Here = new QAction(MainWindow);
        actionNothing_Here->setObjectName(QStringLiteral("actionNothing_Here"));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        verticalLayout_6 = new QVBoxLayout(centralWidget);
        verticalLayout_6->setSpacing(6);
        verticalLayout_6->setContentsMargins(11, 11, 11, 11);
        verticalLayout_6->setObjectName(QStringLiteral("verticalLayout_6"));
        scrollArea = new QScrollArea(centralWidget);
        scrollArea->setObjectName(QStringLiteral("scrollArea"));
        scrollArea->setAutoFillBackground(true);
        scrollArea->setWidgetResizable(false);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName(QStringLiteral("scrollAreaWidgetContents"));
        verticalLayout_5 = new QVBoxLayout(scrollAreaWidgetContents);
        verticalLayout_5->setSpacing(6);
        verticalLayout_5->setContentsMargins(11, 11, 11, 11);
        verticalLayout_5->setObjectName(QStringLiteral("verticalLayout_5"));
        canvas = new Canvas(scrollAreaWidgetContents);
        canvas->setObjectName(QStringLiteral("canvas"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(canvas->sizePolicy().hasHeightForWidth());
        canvas->setSizePolicy(sizePolicy);

        verticalLayout_5->addWidget(canvas);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_5->addItem(verticalSpacer);

        scrollArea->setWidget(scrollAreaWidgetContents);

        verticalLayout_6->addWidget(scrollArea);

        MainWindow->setCentralWidget(centralWidget);
        dockWidget = new QDockWidget(MainWindow);
        dockWidget->setObjectName(QStringLiteral("dockWidget"));
        dockWidget->setFeatures(QDockWidget::AllDockWidgetFeatures);
        dockWidget->setAllowedAreas(Qt::AllDockWidgetAreas);
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QStringLiteral("dockWidgetContents"));
        verticalLayout_4 = new QVBoxLayout(dockWidgetContents);
        verticalLayout_4->setSpacing(6);
        verticalLayout_4->setContentsMargins(11, 11, 11, 11);
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        colorBox = new ColorBox(dockWidgetContents);
        colorBox->setObjectName(QStringLiteral("colorBox"));

        verticalLayout_4->addWidget(colorBox);

        colorGrid = new ColorGrid(dockWidgetContents);
        colorGrid->setObjectName(QStringLiteral("colorGrid"));

        verticalLayout_4->addWidget(colorGrid);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        label = new QLabel(dockWidgetContents);
        label->setObjectName(QStringLiteral("label"));

        horizontalLayout_3->addWidget(label);

        spinBox = new QSpinBox(dockWidgetContents);
        spinBox->setObjectName(QStringLiteral("spinBox"));
        spinBox->setMinimum(1);
        spinBox->setMaximum(100);
        spinBox->setValue(12);

        horizontalLayout_3->addWidget(spinBox);


        verticalLayout_4->addLayout(horizontalLayout_3);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        pencilButton = new QPushButton(dockWidgetContents);
        buttonGroup = new QButtonGroup(MainWindow);
        buttonGroup->setObjectName(QStringLiteral("buttonGroup"));
        buttonGroup->addButton(pencilButton);
        pencilButton->setObjectName(QStringLiteral("pencilButton"));
        pencilButton->setCheckable(true);
        pencilButton->setChecked(true);
        pencilButton->setAutoExclusive(true);

        verticalLayout_3->addWidget(pencilButton);

        brushButton = new QPushButton(dockWidgetContents);
        buttonGroup->addButton(brushButton);
        brushButton->setObjectName(QStringLiteral("brushButton"));
        brushButton->setCheckable(true);
        brushButton->setChecked(false);
        brushButton->setAutoExclusive(true);

        verticalLayout_3->addWidget(brushButton);

        sketchButton = new QPushButton(dockWidgetContents);
        buttonGroup->addButton(sketchButton);
        sketchButton->setObjectName(QStringLiteral("sketchButton"));
        sketchButton->setCheckable(true);
        sketchButton->setAutoExclusive(true);

        verticalLayout_3->addWidget(sketchButton);

        eraserButton = new QPushButton(dockWidgetContents);
        buttonGroup->addButton(eraserButton);
        eraserButton->setObjectName(QStringLiteral("eraserButton"));
        eraserButton->setCheckable(true);
        eraserButton->setAutoExclusive(true);

        verticalLayout_3->addWidget(eraserButton);

        colorPicker = new QPushButton(dockWidgetContents);
        colorPicker->setObjectName(QStringLiteral("colorPicker"));
        colorPicker->setCheckable(true);

        verticalLayout_3->addWidget(colorPicker);


        verticalLayout_4->addLayout(verticalLayout_3);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_4->addItem(verticalSpacer_2);

        dockWidget->setWidget(dockWidgetContents);
        MainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(1), dockWidget);
        dockWidget_2 = new QDockWidget(MainWindow);
        dockWidget_2->setObjectName(QStringLiteral("dockWidget_2"));
        dockWidget_2->setMinimumSize(QSize(200, 149));
        dockWidget_2->setFeatures(QDockWidget::AllDockWidgetFeatures);
        dockWidget_2->setAllowedAreas(Qt::AllDockWidgetAreas);
        dockWidgetContents_2 = new QWidget();
        dockWidgetContents_2->setObjectName(QStringLiteral("dockWidgetContents_2"));
        verticalLayout = new QVBoxLayout(dockWidgetContents_2);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        textEdit = new QTextEdit(dockWidgetContents_2);
        textEdit->setObjectName(QStringLiteral("textEdit"));
        textEdit->setReadOnly(true);
        textEdit->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse|Qt::TextBrowserInteraction|Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        verticalLayout->addWidget(textEdit);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        lineEdit = new QLineEdit(dockWidgetContents_2);
        lineEdit->setObjectName(QStringLiteral("lineEdit"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(1);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(lineEdit->sizePolicy().hasHeightForWidth());
        lineEdit->setSizePolicy(sizePolicy1);

        horizontalLayout->addWidget(lineEdit);

        pushButton = new QPushButton(dockWidgetContents_2);
        pushButton->setObjectName(QStringLiteral("pushButton"));

        horizontalLayout->addWidget(pushButton);


        verticalLayout->addLayout(horizontalLayout);

        dockWidget_2->setWidget(dockWidgetContents_2);
        MainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(2), dockWidget_2);
        dockWidget_3 = new QDockWidget(MainWindow);
        dockWidget_3->setObjectName(QStringLiteral("dockWidget_3"));
        dockWidget_3->setMinimumSize(QSize(200, 117));
        dockWidget_3->setFeatures(QDockWidget::AllDockWidgetFeatures);
        dockWidget_3->setAllowedAreas(Qt::AllDockWidgetAreas);
        dockWidgetContents_3 = new QWidget();
        dockWidgetContents_3->setObjectName(QStringLiteral("dockWidgetContents_3"));
        verticalLayout_2 = new QVBoxLayout(dockWidgetContents_3);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        layerWidget = new LayerWidget(dockWidgetContents_3);
        layerWidget->setObjectName(QStringLiteral("layerWidget"));

        verticalLayout_2->addWidget(layerWidget);

        dockWidget_3->setWidget(dockWidgetContents_3);
        MainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(2), dockWidget_3);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 982, 23));
        menu_File = new QMenu(menuBar);
        menu_File->setObjectName(QStringLiteral("menu_File"));
        menuExport = new QMenu(menu_File);
        menuExport->setObjectName(QStringLiteral("menuExport"));
        menu_Edit = new QMenu(menuBar);
        menu_Edit->setObjectName(QStringLiteral("menu_Edit"));
        menu_About = new QMenu(menuBar);
        menu_About->setObjectName(QStringLiteral("menu_About"));
        menu_View = new QMenu(menuBar);
        menu_View->setObjectName(QStringLiteral("menu_View"));
        MainWindow->setMenuBar(menuBar);

        menuBar->addAction(menu_File->menuAction());
        menuBar->addAction(menu_Edit->menuAction());
        menuBar->addAction(menu_View->menuAction());
        menuBar->addAction(menu_About->menuAction());
        menu_File->addAction(menuExport->menuAction());
        menu_File->addAction(action_Quit);
        menuExport->addAction(actionExport_Visiable);
        menuExport->addAction(actionExport_All);
        menuExport->addAction(actionExport_Visible_To_ClipBorad);
        menuExport->addAction(actionExport_All_To_Clipboard);
        menu_Edit->addAction(actionNothing_Here);
        menu_About->addAction(action_About_Mr_Paint);
        menu_About->addAction(actionAbout_Qt);
        menu_View->addAction(actionReset_View);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "Mr.Paint", 0));
        actionExport_All->setText(QApplication::translate("MainWindow", "Export All to File", 0));
        actionExport_Visiable->setText(QApplication::translate("MainWindow", "Export Visible To File", 0));
        action_Quit->setText(QApplication::translate("MainWindow", "&Quit", 0));
        action_About_Mr_Paint->setText(QApplication::translate("MainWindow", "About Mr.Paint", 0));
        actionAbout_Qt->setText(QApplication::translate("MainWindow", "About Qt", 0));
        actionExport_Visible_To_ClipBorad->setText(QApplication::translate("MainWindow", "Export Visible To ClipBorad", 0));
        actionExport_All_To_Clipboard->setText(QApplication::translate("MainWindow", "Export All To Clipboard", 0));
        actionReset_View->setText(QApplication::translate("MainWindow", "Reset View", 0));
        actionNothing_Here->setText(QApplication::translate("MainWindow", "Nothing Here", 0));
        dockWidget->setWindowTitle(QApplication::translate("MainWindow", "Settings", 0));
        label->setText(QApplication::translate("MainWindow", "Width (Q/W)", 0));
        pencilButton->setText(QApplication::translate("MainWindow", "Pencil (Z)", 0));
        pencilButton->setShortcut(QApplication::translate("MainWindow", "Z", 0));
        brushButton->setText(QApplication::translate("MainWindow", "Brush (A)", 0));
        brushButton->setShortcut(QApplication::translate("MainWindow", "A", 0));
        sketchButton->setText(QApplication::translate("MainWindow", "Sketch (S)", 0));
        sketchButton->setShortcut(QApplication::translate("MainWindow", "S", 0));
        eraserButton->setText(QApplication::translate("MainWindow", "Eraser (E)", 0));
        eraserButton->setShortcut(QApplication::translate("MainWindow", "E", 0));
        colorPicker->setText(QApplication::translate("MainWindow", "Color Picker (C)", 0));
        colorPicker->setShortcut(QApplication::translate("MainWindow", "C", 0));
        dockWidget_2->setWindowTitle(QApplication::translate("MainWindow", "Chat", 0));
        pushButton->setText(QApplication::translate("MainWindow", "Send", 0));
        dockWidget_3->setWindowTitle(QApplication::translate("MainWindow", "Layers", 0));
        menu_File->setTitle(QApplication::translate("MainWindow", "&File", 0));
        menuExport->setTitle(QApplication::translate("MainWindow", "Export...", 0));
        menu_Edit->setTitle(QApplication::translate("MainWindow", "&Edit", 0));
        menu_About->setTitle(QApplication::translate("MainWindow", "&About", 0));
        menu_View->setTitle(QApplication::translate("MainWindow", "&View", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
