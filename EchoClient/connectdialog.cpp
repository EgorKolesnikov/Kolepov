#include <QtWidgets>
#include <QtNetwork>

#include "connectdialog.h"

ConnectDialog::ConnectDialog(QWidget *parent)
    : QDialog(parent)
    , hostCombo(new QComboBox)
    , portLineEdit(new QLineEdit)
    , loginLineEdit(new QLineEdit)
    , connectButton(new QPushButton(tr("Connect")))
    , tcpSocket(new QTcpSocket)
    , connectProgress(new QProgressBar)
{

    this->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    this->setWindowTitle(tr("Connect"));
    hostCombo->setEditable(true);

    // find out name of this machine
    QString name = QHostInfo::localHostName();
    if (!name.isEmpty()) {
        hostCombo->addItem(name);
        QString domain = QHostInfo::localDomainName();
        if (!domain.isEmpty())
            hostCombo->addItem(name + QChar('.') + domain);
    }
    if (name != QLatin1String("localhost"))
        hostCombo->addItem(QString("localhost"));
    // find out IP addresses of this machine
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // add non-localhost addresses
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (!ipAddressesList.at(i).isLoopback())
            hostCombo->addItem(ipAddressesList.at(i).toString());
    }
    // add localhost addresses
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i).isLoopback())
            hostCombo->addItem(ipAddressesList.at(i).toString());
    }

    portLineEdit->setValidator(new QIntValidator(1, 65535, this));
    portLineEdit->setText(QString::number(PORT));

    QLabel *hostLabel = new QLabel(tr("&Server name:"));
    hostLabel->setBuddy(hostCombo);
    QLabel *portLabel = new QLabel(tr("S&erver port:"));
    portLabel->setBuddy(portLineEdit);
    QLabel *loginLabel = new QLabel(tr("&Login:"));
    loginLabel->setBuddy(loginLineEdit);

    connectProgress->setTextVisible(false);

    connectButton->setDefault(true);
    connectButton->setEnabled(false);

    QPushButton *quitButton = new QPushButton(tr("Quit"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    buttonBox->addButton(connectButton, QDialogButtonBox::AcceptRole);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);
    connect(buttonBox, SIGNAL(rejected()),
            SLOT(reject())
            );

    connect(hostCombo, SIGNAL(editTextChanged(QString)),
            SLOT(enableConnectButton())
            );
    connect(portLineEdit, SIGNAL(textChanged(QString)),
            SLOT(enableConnectButton())
            );
    connect(loginLineEdit, SIGNAL(textChanged(QString)),
            SLOT(enableConnectButton())
            );
    connect(connectButton, SIGNAL(clicked(bool)),
            SLOT(connectToServer())
            );
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            SLOT(displayError(QAbstractSocket::SocketError))
            );
    connect(tcpSocket, SIGNAL(connected()),
            SLOT(sendLogin())
            );
    connect(tcpSocket, SIGNAL(readyRead()),
            SLOT(showResult())
            );

    QGroupBox *loginBox = new QGroupBox(tr("Login options")),
            *connectBox = new QGroupBox(tr("Connect options"));
    QFormLayout *loginLayout = new QFormLayout,
            *connectLayout = new QFormLayout;

    loginLayout->addRow(loginLabel, loginLineEdit);
    loginBox->setLayout(loginLayout);

    connectLayout->addRow(hostLabel, hostCombo);
    connectLayout->addRow(portLabel, portLineEdit);
    connectBox->setLayout(connectLayout);

    QGridLayout *mainLayout = new QGridLayout;

    mainLayout->addWidget(loginBox, 0, 0);
    mainLayout->addWidget(connectBox, 0, 1);
    mainLayout->addWidget(connectProgress, 1, 0, 1, 2);
    mainLayout->addWidget(buttonBox, 2, 0, 1, 2);

    this->setLayout(mainLayout);

    setWindowTitle(tr("Connection"));
    loginLineEdit->setFocus();
}

void ConnectDialog::connectToServer()
{
    connectProgress->setRange(0, 0);

    connectButton->setEnabled(false);
    tcpSocket->abort();
    tcpSocket->connectToHost(hostCombo->currentText(),
                             portLineEdit->text().toInt());

}


void ConnectDialog::displayError(QAbstractSocket::SocketError socketError)
{
    connectProgress->setRange(0, 1);

    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(this, tr("Error"),
                                 tr("The host was not found. Please check the "
                                    "host name and port settings."));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::information(this, tr("Error"),
                                 tr("The connection was refused by the peer. "
                                    "Make sure the server is running, "
                                    "and check that the host name and port "
                                    "settings are correct."));
        break;
    default:
        QMessageBox::information(this, tr("Error"),
                                 tr("The following error occurred: %1.")
                                 .arg(tcpSocket->errorString()));
    }

    connectButton->setEnabled(true);
}

void ConnectDialog::enableConnectButton()
{
    connectButton->setEnabled(  !hostCombo->currentText().isEmpty() &&
                                !portLineEdit->text().isEmpty()     &&
                                !loginLineEdit->text().isEmpty()
                                );

}

void ConnectDialog::sendLogin()
{
    connectProgress->setRange(0, 1);
    connectButton->setEnabled(true);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    out << loginLineEdit->text();

    tcpSocket->write(block);
}

void ConnectDialog::showResult()
{
    QString answer;
    QChar ind;

    QDataStream in(tcpSocket);
    in.setVersion(QDataStream::Qt_4_0);

    in.startTransaction();
    if (!in.commitTransaction())
        return;

    in >> ind;
    if (ind == PROTOCOL::LOGIN_OK)
    {
        this->accept();
    }
    else
    {
        in >> answer;
        QMessageBox::information(this, "Server Response", answer);
    }

}

