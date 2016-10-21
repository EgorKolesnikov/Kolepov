#include <QtWidgets>
#include <QtNetwork>

#include "connectdialog.h"

ConnectDialog::ConnectDialog(QWidget *parent)
    : QDialog(parent)
    , hostCombo(new QComboBox)
    , portLineEdit(new QLineEdit)
    , loginLineEdit(new QLineEdit)
    , passwordLineEdit(new QLineEdit)
    , keyPathLineEdit (new QLineEdit)
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

    passwordLineEdit->setEchoMode(QLineEdit::Password);
    passwordLineEdit->setInputMethodHints(Qt::ImhHiddenText| Qt::ImhNoPredictiveText|Qt::ImhNoAutoUppercase);

    QLabel *hostLabel = new QLabel(tr("&Server name:"));
    hostLabel->setBuddy(hostCombo);
    QLabel *portLabel = new QLabel(tr("S&erver port:"));
    portLabel->setBuddy(portLineEdit);
    QLabel *loginLabel = new QLabel(tr("&Login:"));
    loginLabel->setBuddy(loginLineEdit);
    QLabel *passwordLabel = new QLabel(tr("&Password:"));
    passwordLabel->setBuddy(passwordLineEdit);
    QLabel *keyFileLabel = new QLabel(tr("Key file:"));
    QPushButton *browse = new QPushButton(tr("Browse..."));
    keyPathLineEdit->setReadOnly(true);

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
    connect(passwordLineEdit, SIGNAL(textChanged(QString)),
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
    connect(browse, SIGNAL(clicked(bool)),
            SLOT(browseKeyFile())
            );

    QGroupBox *loginBox = new QGroupBox(tr("Login options")),
            *connectBox = new QGroupBox(tr("Connect options"));
    QFormLayout *loginLayout = new QFormLayout,
            *connectLayout = new QFormLayout;

    loginLayout->addRow(loginLabel, loginLineEdit);
    loginLayout->addRow(passwordLabel, passwordLineEdit);
    loginLayout->addWidget(keyFileLabel);
    loginLayout->addRow(browse, keyPathLineEdit);
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

void ConnectDialog::browseKeyFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select key file"));
    if (!fileName.isNull())
    {
        keyPathLineEdit->setText(fileName);
        enableConnectButton();
    }
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
                                !loginLineEdit->text().isEmpty()    &&
                                !passwordLineEdit->text().isEmpty() &&
                                !keyPathLineEdit->text().isEmpty()
                                );

}

void ConnectDialog::sendLogin()
{
    //read server_pk from file and decode from base64
    AutoSeededRandomPool rng;
    FileSource file("server_pk", true, new Base64Decoder);
    RSA::PublicKey serverPK;
    serverPK.BERDecode(file);

    //generate session key
    SecByteBlock key(AES::MAX_KEYLENGTH);
    rng.GenerateBlock(key, key.size());

    qDebug() << QByteArray((char*)key.data(), key.size()).toHex();

    //encode session key with server pk
    std::string encSessionKey;
    RSAES_OAEP_SHA_Encryptor e(serverPK);
    StringSource(key, key.size(), true,
          new PK_EncryptorFilter(rng, e,
                new StringSink(encSessionKey)
          )
    );

    //send encrypted session key
    QByteArray data(encSessionKey.c_str(), encSessionKey.length());
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    out << data << loginLineEdit->text();
    tcpSocket->write(block);
}

void ConnectDialog::showResult()
{
    connectProgress->setRange(0, 1);
    connectButton->setEnabled(true);

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

SecByteBlock ConnectDialog::getHashFromPassword(const QString& password)
{
    SHA256 hash;
    byte aes_key[SHA256::DIGESTSIZE];
    memset(aes_key, 0, SHA256::DIGESTSIZE);
    QByteArray password_bytes = password.toUtf8();
    hash.Update((byte*)password_bytes.constData(), password_bytes.length());
    hash.Final(aes_key);
    SecByteBlock key(aes_key, SHA256::DIGESTSIZE);
    memset(aes_key, 0, SHA256::DIGESTSIZE);

    return key;
}

void ConnectDialog::makeSecureConnection()
{
    //read server_pk from file and decode from base64
    AutoSeededRandomPool rng;
    FileSource file("server_pk", true, new Base64Decoder);
    RSA::PublicKey serverPK;
    serverPK.BERDecode(file);

    //generate session key
    SecByteBlock key(AES::MAX_KEYLENGTH);
    rng.GenerateBlock(key, key.size());

    qDebug() << QByteArray((char*)key.data(), key.size()).toHex();

    //encode session key with server pk
    std::string encSessionKey;
    RSAES_OAEP_SHA_Encryptor e(serverPK);
    StringSource(key, key.size(), true,
          new PK_EncryptorFilter(rng, e,
                new StringSink(encSessionKey)
          )
    );

    //send encrypted session key
    QByteArray data(encSessionKey.c_str(), encSessionKey.length());
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    out << data;

    tcpSocket->write(block);


}
