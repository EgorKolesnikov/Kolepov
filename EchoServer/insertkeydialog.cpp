#include <QtWidgets>
#include <QtNetwork>

#include "insertkeydialog.h"

InsertKeyDialog::InsertKeyDialog(QWidget *parent)
    : QDialog(parent)
    , keyPathLineEdit (new QLineEdit)
    , connectButton (new QPushButton(tr("Run server")))
{
    QLabel *keyFileLabel = new QLabel(tr("Key path:"));
    QPushButton *browse = new QPushButton(tr("Browse..."));
    keyPathLineEdit->setReadOnly(true);
    connectButton->setEnabled(false);

    connect(browse, SIGNAL(clicked(bool)),
            SLOT(browseServerKeyFile())
            );
    connect(connectButton, SIGNAL(clicked(bool)),
            SLOT(runServer())
            );


    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(keyFileLabel);
    mainLayout->addWidget(browse);
    mainLayout->addWidget(keyPathLineEdit);
    mainLayout->addWidget(connectButton);
    this->setLayout(mainLayout);
}

void InsertKeyDialog::browseServerKeyFile(){
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select key file"));
    if (!fileName.isNull())
    {
        QStorageInfo storage = QStorageInfo(fileName);
        if(storage.fileSystemType() != "fuseblk" && storage.fileSystemType() != "FAT32"){
            QMessageBox msgBox;
            msgBox.setText("Error");
            msgBox.setInformativeText("Invalid device.");
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.exec();
        }
        else{
            keyPathLineEdit->setText(fileName);
            connectButton->setEnabled(true);
        }
    }
}

void InsertKeyDialog::runServer(){
    this->accept();
}

QString InsertKeyDialog::getChoosenPath(){
    return keyPathLineEdit->text();
}
