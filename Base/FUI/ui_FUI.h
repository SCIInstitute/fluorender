/********************************************************************************
** Form generated from reading UI file 'FUI.ui'
**
** Created by: Qt User Interface Compiler version 5.12.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FUI_H
#define UI_FUI_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_FUI
{
public:
    QWidget *centralWidget;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;
    QDockWidget *dockWidget_7;
    QWidget *dockWidgetContents_7;
    QGridLayout *gridLayout_2;
    QTextBrowser *textBrowser;
    QDockWidget *dockWidget_8;
    QWidget *dockWidgetContents_8;
    QDockWidget *dockWidget_9;
    QWidget *dockWidgetContents_9;
    QGridLayout *gridLayout;
    QTextBrowser *textBrowser_2;
    QDockWidget *dockWidget_10;
    QWidget *dockWidgetContents_10;
    QGridLayout *gridLayout_3;
    QTextBrowser *textBrowser_3;

    void setupUi(QMainWindow *FUI)
    {
        if (FUI->objectName().isEmpty())
            FUI->setObjectName(QString::fromUtf8("FUI"));
        FUI->resize(1805, 929);
        FUI->setLayoutDirection(Qt::RightToLeft);
        centralWidget = new QWidget(FUI);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        FUI->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(FUI);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1805, 21));
        FUI->setMenuBar(menuBar);
        mainToolBar = new QToolBar(FUI);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        FUI->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(FUI);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        FUI->setStatusBar(statusBar);
        dockWidget_7 = new QDockWidget(FUI);
        dockWidget_7->setObjectName(QString::fromUtf8("dockWidget_7"));
        dockWidget_7->setAcceptDrops(true);
        dockWidgetContents_7 = new QWidget();
        dockWidgetContents_7->setObjectName(QString::fromUtf8("dockWidgetContents_7"));
        gridLayout_2 = new QGridLayout(dockWidgetContents_7);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        textBrowser = new QTextBrowser(dockWidgetContents_7);
        textBrowser->setObjectName(QString::fromUtf8("textBrowser"));

        gridLayout_2->addWidget(textBrowser, 0, 0, 1, 1);

        dockWidget_7->setWidget(dockWidgetContents_7);
        FUI->addDockWidget(static_cast<Qt::DockWidgetArea>(1), dockWidget_7);
        dockWidget_8 = new QDockWidget(FUI);
        dockWidget_8->setObjectName(QString::fromUtf8("dockWidget_8"));
        dockWidget_8->setAcceptDrops(false);
        dockWidget_8->setAllowedAreas(Qt::BottomDockWidgetArea);
        dockWidgetContents_8 = new QWidget();
        dockWidgetContents_8->setObjectName(QString::fromUtf8("dockWidgetContents_8"));
        dockWidget_8->setWidget(dockWidgetContents_8);
        FUI->addDockWidget(static_cast<Qt::DockWidgetArea>(8), dockWidget_8);
        dockWidget_9 = new QDockWidget(FUI);
        dockWidget_9->setObjectName(QString::fromUtf8("dockWidget_9"));
        dockWidgetContents_9 = new QWidget();
        dockWidgetContents_9->setObjectName(QString::fromUtf8("dockWidgetContents_9"));
        gridLayout = new QGridLayout(dockWidgetContents_9);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        textBrowser_2 = new QTextBrowser(dockWidgetContents_9);
        textBrowser_2->setObjectName(QString::fromUtf8("textBrowser_2"));

        gridLayout->addWidget(textBrowser_2, 0, 0, 1, 1);

        dockWidget_9->setWidget(dockWidgetContents_9);
        FUI->addDockWidget(static_cast<Qt::DockWidgetArea>(1), dockWidget_9);
        dockWidget_10 = new QDockWidget(FUI);
        dockWidget_10->setObjectName(QString::fromUtf8("dockWidget_10"));
        dockWidgetContents_10 = new QWidget();
        dockWidgetContents_10->setObjectName(QString::fromUtf8("dockWidgetContents_10"));
        gridLayout_3 = new QGridLayout(dockWidgetContents_10);
        gridLayout_3->setSpacing(6);
        gridLayout_3->setContentsMargins(11, 11, 11, 11);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        textBrowser_3 = new QTextBrowser(dockWidgetContents_10);
        textBrowser_3->setObjectName(QString::fromUtf8("textBrowser_3"));

        gridLayout_3->addWidget(textBrowser_3, 0, 0, 1, 1);

        dockWidget_10->setWidget(dockWidgetContents_10);
        FUI->addDockWidget(static_cast<Qt::DockWidgetArea>(1), dockWidget_10);

        retranslateUi(FUI);

        QMetaObject::connectSlotsByName(FUI);
    } // setupUi

    void retranslateUi(QMainWindow *FUI)
    {
        FUI->setWindowTitle(QApplication::translate("FUI", "FUI", nullptr));
    } // retranslateUi

};

namespace Ui {
    class FUI: public Ui_FUI {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FUI_H
