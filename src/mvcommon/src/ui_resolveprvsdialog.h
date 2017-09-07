/********************************************************************************
** Form generated from reading UI file 'resolveprvsdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_RESOLVEPRVSDIALOG_H
#define UI_RESOLVEPRVSDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_ResolvePrvsDialog
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QSpacerItem *verticalSpacer_2;
    QLabel *label_2;
    QRadioButton *open_prv_gui;
    QRadioButton *automatically_download;
    QRadioButton *proceed_anyway;
    QSpacerItem *verticalSpacer;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *ResolvePrvsDialog)
    {
        if (ResolvePrvsDialog->objectName().isEmpty())
            ResolvePrvsDialog->setObjectName(QStringLiteral("ResolvePrvsDialog"));
        ResolvePrvsDialog->resize(928, 399);
        QFont font;
        font.setPointSize(14);
        ResolvePrvsDialog->setFont(font);
        verticalLayout = new QVBoxLayout(ResolvePrvsDialog);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        label = new QLabel(ResolvePrvsDialog);
        label->setObjectName(QStringLiteral("label"));
        label->setFont(font);
        label->setWordWrap(true);

        verticalLayout->addWidget(label);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer_2);

        label_2 = new QLabel(ResolvePrvsDialog);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setFont(font);

        verticalLayout->addWidget(label_2);

        open_prv_gui = new QRadioButton(ResolvePrvsDialog);
        open_prv_gui->setObjectName(QStringLiteral("open_prv_gui"));
        open_prv_gui->setChecked(true);

        verticalLayout->addWidget(open_prv_gui);

        automatically_download = new QRadioButton(ResolvePrvsDialog);
        automatically_download->setObjectName(QStringLiteral("automatically_download"));

        verticalLayout->addWidget(automatically_download);

        proceed_anyway = new QRadioButton(ResolvePrvsDialog);
        proceed_anyway->setObjectName(QStringLiteral("proceed_anyway"));

        verticalLayout->addWidget(proceed_anyway);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        buttonBox = new QDialogButtonBox(ResolvePrvsDialog);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(ResolvePrvsDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), ResolvePrvsDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), ResolvePrvsDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(ResolvePrvsDialog);
    } // setupUi

    void retranslateUi(QDialog *ResolvePrvsDialog)
    {
        ResolvePrvsDialog->setWindowTitle(QApplication::translate("ResolvePrvsDialog", "Resolving .prv objects", 0));
        label->setText(QApplication::translate("ResolvePrvsDialog", "Not all .prv objects could be resolved. This means that not all of the referenced files are on your computer. If they are on a server then you may be able to download them to your machine, assuming those servers are referenced in your mountainlab configuration file. These files may also be regenerated if the raw data are on your computer. If you proceed without downloading or regenerating files, then the GUI will be opened with only partial access to the data.", 0));
        label_2->setText(QApplication::translate("ResolvePrvsDialog", "What would you like to do?", 0));
        open_prv_gui->setText(QApplication::translate("ResolvePrvsDialog", "Open prv-gui to try to download and/or regenerate the referenced data files", 0));
        automatically_download->setText(QApplication::translate("ResolvePrvsDialog", "Try to automatically download files from configured servers and/or regenerate them", 0));
        proceed_anyway->setText(QApplication::translate("ResolvePrvsDialog", "Proceed without downloading or regenerating any files", 0));
    } // retranslateUi

};

namespace Ui {
    class ResolvePrvsDialog: public Ui_ResolvePrvsDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_RESOLVEPRVSDIALOG_H
