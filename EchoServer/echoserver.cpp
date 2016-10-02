#include <QtWidgets>
#include <QtNetwork>

#include <stdlib.h>

#include "echoserver.h"
#include "serverthread.h"
#include "sqlwrapper.h"

EchoServer::EchoServer(QWidget *parent)
    : QWidget(parent)
    , statusLabel(new QLabel)
    , tcpServer(Q_NULLPTR)
    , connections(new QTextEdit)
    , database(new SqlWrapper(this))
{
    sessionOpened();

    users << tr("Egor")
          << tr("Denis");

    QPushButton *quitButton = new QPushButton(tr("Quit"));
    connections->setReadOnly(true);

    connect(quitButton, SIGNAL(clicked(bool)),
            SLOT(close())
            );
    connect(tcpServer, SIGNAL(newConnection()),
            SLOT(replyToClient())
            );

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(quitButton);
    buttonLayout->addStretch(1);

    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addWidget(statusLabel);
    mainLayout->addWidget(connections);
    mainLayout->addLayout(buttonLayout);

    this->setLayout(mainLayout);

    setWindowTitle(QGuiApplication::applicationDisplayName());
}

void EchoServer::sessionOpened()
{
    tcpServer = new QTcpServer(this);
    if (!tcpServer->listen()) {
        QMessageBox::critical(this, tr("Server"),
                              tr("Unable to start the server: %1.")
                              .arg(tcpServer->errorString()));
        close();
        return;
    }
    QString ipAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // use the first non-localhost IPv4 address
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (!ipAddressesList.at(i).isLoopback() &&
            ipAddressesList.at(i).toIPv4Address()) {
            ipAddress = ipAddressesList.at(i).toString();
            break;
        }
    }
    // if we did not find one, use IPv4 localhost
    if (ipAddress.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
    statusLabel->setText(tr("The server is running on\n\nIP: %1\nport: %2\n\n")
                         .arg(ipAddress).arg(tcpServer->serverPort()));
}

void EchoServer::replyToClient()
{
    QTcpSocket *clientConnection = tcpServer->nextPendingConnection();
    ServerThread *thread = new ServerThread(clientConnection->socketDescriptor(),
                                            users,
                                            this);
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(clientConnection, SIGNAL(disconnected()),
            clientConnection, SLOT(deleteLater()));

    thread->start();

}
