/********************************************************************************
** Form generated from reading UI file 'widget.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Widget
{
public:
    QPushButton *newgameButton;
    QLabel *whitePiecesLabel;
    QLabel *blackPiecesLabel;
    QLabel *whiteCapturedLabel;
    QLabel *blackCapturedLabel;
    QLabel *stepLabel;
    QPushButton *resetgameButton;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName("Widget");
        Widget->resize(871, 600);
        newgameButton = new QPushButton(Widget);
        newgameButton->setObjectName("newgameButton");
        newgameButton->setGeometry(QRect(640, 50, 93, 29));
        whitePiecesLabel = new QLabel(Widget);
        whitePiecesLabel->setObjectName("whitePiecesLabel");
        whitePiecesLabel->setGeometry(QRect(640, 130, 121, 41));
        blackPiecesLabel = new QLabel(Widget);
        blackPiecesLabel->setObjectName("blackPiecesLabel");
        blackPiecesLabel->setGeometry(QRect(640, 170, 121, 21));
        whiteCapturedLabel = new QLabel(Widget);
        whiteCapturedLabel->setObjectName("whiteCapturedLabel");
        whiteCapturedLabel->setGeometry(QRect(640, 200, 221, 21));
        blackCapturedLabel = new QLabel(Widget);
        blackCapturedLabel->setObjectName("blackCapturedLabel");
        blackCapturedLabel->setGeometry(QRect(640, 230, 211, 21));
        stepLabel = new QLabel(Widget);
        stepLabel->setObjectName("stepLabel");
        stepLabel->setGeometry(QRect(640, 260, 111, 21));
        resetgameButton = new QPushButton(Widget);
        resetgameButton->setObjectName("resetgameButton");
        resetgameButton->setGeometry(QRect(640, 90, 93, 29));

        retranslateUi(Widget);

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QCoreApplication::translate("Widget", "Widget", nullptr));
        newgameButton->setText(QCoreApplication::translate("Widget", "New Game", nullptr));
        whitePiecesLabel->setText(QCoreApplication::translate("Widget", "White Pieces: 16", nullptr));
        blackPiecesLabel->setText(QCoreApplication::translate("Widget", "Black Pieces: 16", nullptr));
        whiteCapturedLabel->setText(QCoreApplication::translate("Widget", "White Pieces Knocked Out: 0", nullptr));
        blackCapturedLabel->setText(QCoreApplication::translate("Widget", "Black Pieces Knocked Out: 0", nullptr));
        stepLabel->setText(QCoreApplication::translate("Widget", "Steps Count: 0", nullptr));
        resetgameButton->setText(QCoreApplication::translate("Widget", "Reset Game", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H
