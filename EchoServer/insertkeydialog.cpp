#include <QtWidgets>
#include <QtNetwork>

#include "insertkeydialog.h"

InsertKeyDialog::InsertKeyDialog(QWidget *parent)
    : QDialog(parent)
    , keyPathLineEdit (new QLineEdit)
    , keyDatabasePathLineEdit(new QLineEdit)
    , connectButton (new QPushButton(tr("Run server")))
{
    QLabel *keyFileLabel = new QLabel(tr("Server key path:"));
    QPushButton *browseServerKey = new QPushButton(tr("Browse..."));
    QLabel *keyDatabaseFileLabel = new QLabel(tr("Database password:"));
    keyPathLineEdit->setReadOnly(true);
    connectButton->setEnabled(false);

    connect(browseServerKey, SIGNAL(clicked(bool)),
            SLOT(browseServerKeyFile())
            );
    connect(connectButton, SIGNAL(clicked(bool)),
            SLOT(runServer())
            );


    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(keyFileLabel);
    mainLayout->addWidget(browseServerKey);
    mainLayout->addWidget(keyPathLineEdit);
    mainLayout->addWidget(keyDatabaseFileLabel);
    mainLayout->addWidget(keyDatabasePathLineEdit);
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

QString InsertKeyDialog::getChoosenServerKeyPath(){
    return keyPathLineEdit->text();
}

QString InsertKeyDialog::getChoosenDatabaseKeyPath(){
    return keyDatabasePathLineEdit->text();
}

void InsertKeyDialog::enableConnectButton(){
    connectButton->setEnabled(!keyDatabasePathLineEdit->text().isEmpty());
}
