/********************************************************************************
** Form generated from reading UI file 'VulnusEditor.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VULNUSEDITOR_H
#define UI_VULNUSEDITOR_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QSlider>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_VulnusEditorClass
{
public:
    QAction *actionExit;
    QAction *actionPaintWound;
    QAction *actionOpen;
    QAction *actionOpen_2;
    QWidget *centralWidget;
    QGroupBox *woundTimelineBox;
    QSlider *timelineSlider;
    QLabel *cacheProgressBar;
    QLabel *woundTimelineLabel;
    QWidget *mainViewLabel;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuTools;

    void setupUi(QMainWindow *VulnusEditorClass)
    {
        if (VulnusEditorClass->objectName().isEmpty())
            VulnusEditorClass->setObjectName(QStringLiteral("VulnusEditorClass"));
        VulnusEditorClass->resize(661, 692);
        VulnusEditorClass->setDocumentMode(false);
        actionExit = new QAction(VulnusEditorClass);
        actionExit->setObjectName(QStringLiteral("actionExit"));
        actionPaintWound = new QAction(VulnusEditorClass);
        actionPaintWound->setObjectName(QStringLiteral("actionPaintWound"));
        actionPaintWound->setCheckable(true);
        actionPaintWound->setChecked(true);
        actionOpen = new QAction(VulnusEditorClass);
        actionOpen->setObjectName(QStringLiteral("actionOpen"));
        actionOpen_2 = new QAction(VulnusEditorClass);
        actionOpen_2->setObjectName(QStringLiteral("actionOpen_2"));
        centralWidget = new QWidget(VulnusEditorClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        woundTimelineBox = new QGroupBox(centralWidget);
        woundTimelineBox->setObjectName(QStringLiteral("woundTimelineBox"));
        woundTimelineBox->setGeometry(QRect(10, 500, 641, 161));
        timelineSlider = new QSlider(woundTimelineBox);
        timelineSlider->setObjectName(QStringLiteral("timelineSlider"));
        timelineSlider->setGeometry(QRect(10, 130, 621, 19));
        timelineSlider->setMaximum(199);
        timelineSlider->setSingleStep(1);
        timelineSlider->setOrientation(Qt::Horizontal);
        timelineSlider->setInvertedAppearance(false);
        timelineSlider->setInvertedControls(false);
        timelineSlider->setTickPosition(QSlider::NoTicks);
        timelineSlider->setTickInterval(0);
        cacheProgressBar = new QLabel(woundTimelineBox);
        cacheProgressBar->setObjectName(QStringLiteral("cacheProgressBar"));
        cacheProgressBar->setGeometry(QRect(10, 140, 621, 11));
        woundTimelineLabel = new QLabel(woundTimelineBox);
        woundTimelineLabel->setObjectName(QStringLiteral("woundTimelineLabel"));
        woundTimelineLabel->setGeometry(QRect(10, 20, 621, 101));
        woundTimelineLabel->setPixmap(QPixmap(QString::fromUtf8(":/VulnusEditor/Wound_healing_phases.png")));
        woundTimelineLabel->setScaledContents(true);
        mainViewLabel = new QWidget(centralWidget);
        mainViewLabel->setObjectName(QStringLiteral("mainViewLabel"));
        mainViewLabel->setGeometry(QRect(10, 10, 640, 480));
        VulnusEditorClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(VulnusEditorClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 661, 21));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QStringLiteral("menuFile"));
        menuTools = new QMenu(menuBar);
        menuTools->setObjectName(QStringLiteral("menuTools"));
        VulnusEditorClass->setMenuBar(menuBar);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuTools->menuAction());
        menuFile->addAction(actionExit);
        menuTools->addAction(actionOpen_2);

        retranslateUi(VulnusEditorClass);

        QMetaObject::connectSlotsByName(VulnusEditorClass);
    } // setupUi

    void retranslateUi(QMainWindow *VulnusEditorClass)
    {
        VulnusEditorClass->setWindowTitle(QApplication::translate("VulnusEditorClass", "Vulnus - (c)2015 Krien Linnenbank", 0));
        actionExit->setText(QApplication::translate("VulnusEditorClass", "Exit", 0));
        actionPaintWound->setText(QApplication::translate("VulnusEditorClass", "Paint wound", 0));
        actionOpen->setText(QApplication::translate("VulnusEditorClass", "Open", 0));
        actionOpen_2->setText(QApplication::translate("VulnusEditorClass", "Open", 0));
        woundTimelineBox->setTitle(QApplication::translate("VulnusEditorClass", "Wound timeline", 0));
        cacheProgressBar->setText(QString());
        woundTimelineLabel->setText(QString());
        menuFile->setTitle(QApplication::translate("VulnusEditorClass", "File", 0));
        menuTools->setTitle(QApplication::translate("VulnusEditorClass", "Tools", 0));
    } // retranslateUi

};

namespace Ui {
    class VulnusEditorClass: public Ui_VulnusEditorClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VULNUSEDITOR_H
