/********************************************************************************
** Form generated from reading UI file 'diffwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIFFWINDOW_H
#define UI_DIFFWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DiffWindow
{
public:
    QHBoxLayout *horizontalLayout_2;
    QPlainTextEdit *plainTextEdit;
    QPlainTextEdit *plainTextEdit_2;

    void setupUi(QWidget *DiffWindow)
    {
        if (DiffWindow->objectName().isEmpty())
            DiffWindow->setObjectName(QString::fromUtf8("DiffWindow"));
        DiffWindow->resize(400, 300);
        horizontalLayout_2 = new QHBoxLayout(DiffWindow);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        plainTextEdit = new QPlainTextEdit(DiffWindow);
        plainTextEdit->setObjectName(QString::fromUtf8("plainTextEdit"));

        horizontalLayout_2->addWidget(plainTextEdit);

        plainTextEdit_2 = new QPlainTextEdit(DiffWindow);
        plainTextEdit_2->setObjectName(QString::fromUtf8("plainTextEdit_2"));

        horizontalLayout_2->addWidget(plainTextEdit_2);


        retranslateUi(DiffWindow);

        QMetaObject::connectSlotsByName(DiffWindow);
    } // setupUi

    void retranslateUi(QWidget *DiffWindow)
    {
        DiffWindow->setWindowTitle(QCoreApplication::translate("DiffWindow", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DiffWindow: public Ui_DiffWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIFFWINDOW_H
