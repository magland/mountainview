/********************************************************************************
** Form generated from reading UI file 'prvmanagerdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PRVMANAGERDIALOG_H
#define UI_PRVMANAGERDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_PrvManagerDialog
{
public:
    QVBoxLayout *verticalLayout;
    QTreeWidget *tree;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer;
    QPushButton *btn_copy_to_local_disk;
    QPushButton *btn_upload_to_server;
    QPushButton *btn_done;

    void setupUi(QDialog *PrvManagerDialog)
    {
        if (PrvManagerDialog->objectName().isEmpty())
            PrvManagerDialog->setObjectName(QStringLiteral("PrvManagerDialog"));
        PrvManagerDialog->resize(647, 321);
        verticalLayout = new QVBoxLayout(PrvManagerDialog);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        tree = new QTreeWidget(PrvManagerDialog);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(0, QStringLiteral("1"));
        tree->setHeaderItem(__qtreewidgetitem);
        tree->setObjectName(QStringLiteral("tree"));

        verticalLayout->addWidget(tree);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);

        btn_copy_to_local_disk = new QPushButton(PrvManagerDialog);
        btn_copy_to_local_disk->setObjectName(QStringLiteral("btn_copy_to_local_disk"));

        horizontalLayout_2->addWidget(btn_copy_to_local_disk);

        btn_upload_to_server = new QPushButton(PrvManagerDialog);
        btn_upload_to_server->setObjectName(QStringLiteral("btn_upload_to_server"));

        horizontalLayout_2->addWidget(btn_upload_to_server);

        btn_done = new QPushButton(PrvManagerDialog);
        btn_done->setObjectName(QStringLiteral("btn_done"));

        horizontalLayout_2->addWidget(btn_done);


        verticalLayout->addLayout(horizontalLayout_2);


        retranslateUi(PrvManagerDialog);

        QMetaObject::connectSlotsByName(PrvManagerDialog);
    } // setupUi

    void retranslateUi(QDialog *PrvManagerDialog)
    {
        PrvManagerDialog->setWindowTitle(QApplication::translate("PrvManagerDialog", "Dialog", 0));
        btn_copy_to_local_disk->setText(QApplication::translate("PrvManagerDialog", "Copy to local disk", 0));
        btn_upload_to_server->setText(QApplication::translate("PrvManagerDialog", "Upload to server", 0));
        btn_done->setText(QApplication::translate("PrvManagerDialog", "Done", 0));
    } // retranslateUi

};

namespace Ui {
    class PrvManagerDialog: public Ui_PrvManagerDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PRVMANAGERDIALOG_H
