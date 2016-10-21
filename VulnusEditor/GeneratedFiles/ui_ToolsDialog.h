/********************************************************************************
** Form generated from reading UI file 'ToolsDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TOOLSDIALOG_H
#define UI_TOOLSDIALOG_H

#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>

QT_BEGIN_NAMESPACE

class Ui_Tools
{
public:
    QGroupBox *paintingGroupbox;
    QCheckBox *paintWoundCheckbox;
    QGroupBox *tessellationGroupbox;
    QGroupBox *cullmodeGroupbox;
    QCheckBox *cullmodeCheckbox;
    QGroupBox *displacementGroupBox;
    QDoubleSpinBox *displacementSpinbox;
    QGroupBox *tessellationFactorGroupbox;
    QSlider *tessFactorSlider;
    QLabel *tessFactorLabel;
    QGroupBox *renderModeGroupbox;
    QPushButton *buttonSolid;
    QPushButton *buttonWireframe;

    void setupUi(QDialog *Tools)
    {
        if (Tools->objectName().isEmpty())
            Tools->setObjectName(QStringLiteral("Tools"));
        Tools->resize(371, 221);
        paintingGroupbox = new QGroupBox(Tools);
        paintingGroupbox->setObjectName(QStringLiteral("paintingGroupbox"));
        paintingGroupbox->setGeometry(QRect(10, 10, 101, 51));
        paintWoundCheckbox = new QCheckBox(paintingGroupbox);
        paintWoundCheckbox->setObjectName(QStringLiteral("paintWoundCheckbox"));
        paintWoundCheckbox->setGeometry(QRect(10, 20, 81, 17));
        paintWoundCheckbox->setChecked(false);
        tessellationGroupbox = new QGroupBox(Tools);
        tessellationGroupbox->setObjectName(QStringLiteral("tessellationGroupbox"));
        tessellationGroupbox->setGeometry(QRect(10, 70, 351, 141));
        cullmodeGroupbox = new QGroupBox(tessellationGroupbox);
        cullmodeGroupbox->setObjectName(QStringLiteral("cullmodeGroupbox"));
        cullmodeGroupbox->setGeometry(QRect(250, 20, 91, 51));
        cullmodeCheckbox = new QCheckBox(cullmodeGroupbox);
        cullmodeCheckbox->setObjectName(QStringLiteral("cullmodeCheckbox"));
        cullmodeCheckbox->setGeometry(QRect(10, 20, 51, 17));
        cullmodeCheckbox->setChecked(true);
        cullmodeCheckbox->setTristate(false);
        displacementGroupBox = new QGroupBox(tessellationGroupbox);
        displacementGroupBox->setObjectName(QStringLiteral("displacementGroupBox"));
        displacementGroupBox->setGeometry(QRect(170, 20, 81, 51));
        displacementSpinbox = new QDoubleSpinBox(displacementGroupBox);
        displacementSpinbox->setObjectName(QStringLiteral("displacementSpinbox"));
        displacementSpinbox->setEnabled(false);
        displacementSpinbox->setGeometry(QRect(10, 20, 51, 22));
        displacementSpinbox->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        displacementSpinbox->setMaximum(5);
        displacementSpinbox->setSingleStep(0.05);
        displacementSpinbox->setValue(0);
        tessellationFactorGroupbox = new QGroupBox(tessellationGroupbox);
        tessellationFactorGroupbox->setObjectName(QStringLiteral("tessellationFactorGroupbox"));
        tessellationFactorGroupbox->setGeometry(QRect(10, 80, 331, 51));
        tessFactorSlider = new QSlider(tessellationFactorGroupbox);
        tessFactorSlider->setObjectName(QStringLiteral("tessFactorSlider"));
        tessFactorSlider->setEnabled(false);
        tessFactorSlider->setGeometry(QRect(40, 20, 281, 22));
        tessFactorSlider->setMinimum(1);
        tessFactorSlider->setMaximum(64);
        tessFactorSlider->setSingleStep(1);
        tessFactorSlider->setPageStep(2);
        tessFactorSlider->setOrientation(Qt::Horizontal);
        tessFactorLabel = new QLabel(tessellationFactorGroupbox);
        tessFactorLabel->setObjectName(QStringLiteral("tessFactorLabel"));
        tessFactorLabel->setGeometry(QRect(10, 20, 16, 20));
        tessFactorLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        renderModeGroupbox = new QGroupBox(tessellationGroupbox);
        renderModeGroupbox->setObjectName(QStringLiteral("renderModeGroupbox"));
        renderModeGroupbox->setGeometry(QRect(10, 20, 161, 51));
        buttonSolid = new QPushButton(renderModeGroupbox);
        buttonSolid->setObjectName(QStringLiteral("buttonSolid"));
        buttonSolid->setGeometry(QRect(8, 20, 75, 23));
        buttonSolid->setCheckable(true);
        buttonSolid->setChecked(true);
        buttonSolid->setFlat(false);
        buttonWireframe = new QPushButton(renderModeGroupbox);
        buttonWireframe->setObjectName(QStringLiteral("buttonWireframe"));
        buttonWireframe->setGeometry(QRect(78, 20, 75, 23));
        buttonWireframe->setCheckable(true);
        buttonWireframe->setChecked(false);
        buttonWireframe->setDefault(false);
        buttonWireframe->setFlat(false);

        retranslateUi(Tools);

        QMetaObject::connectSlotsByName(Tools);
    } // setupUi

    void retranslateUi(QDialog *Tools)
    {
        Tools->setWindowTitle(QApplication::translate("Tools", "Tools", 0));
        paintingGroupbox->setTitle(QApplication::translate("Tools", "Painting", 0));
        paintWoundCheckbox->setText(QApplication::translate("Tools", "Paint wound", 0));
        tessellationGroupbox->setTitle(QApplication::translate("Tools", "Tessellation", 0));
        cullmodeGroupbox->setTitle(QApplication::translate("Tools", "Backface culling", 0));
        cullmodeCheckbox->setText(QApplication::translate("Tools", "Enable", 0));
        displacementGroupBox->setTitle(QApplication::translate("Tools", "Displacement", 0));
        displacementSpinbox->setSuffix(QApplication::translate("Tools", "f", 0));
        tessellationFactorGroupbox->setTitle(QApplication::translate("Tools", "Tessellation factor", 0));
        tessFactorLabel->setText(QApplication::translate("Tools", "99", 0));
        renderModeGroupbox->setTitle(QApplication::translate("Tools", "Rendermode", 0));
        buttonSolid->setText(QApplication::translate("Tools", "Solid", 0));
        buttonWireframe->setText(QApplication::translate("Tools", "Wireframe", 0));
    } // retranslateUi

};

namespace Ui {
    class Tools: public Ui_Tools {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TOOLSDIALOG_H
