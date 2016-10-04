#include <QtWidgets>
#include <QtNetwork>

#include <stdlib.h>

#include "echoserver.h"
#include "serverthread.h"
#include "sqlwrapper.h"
#include "protocol.h"


EchoServer::EchoServer(QWidget *parent)
    : QWidget(parent)
    , tcpServer(Q_NULLPTR)
    , connections(new QTextEdit)
    , database(new SqlWrapper(this))
{
    QLabel *statusLabel = new QLabel;
    sessionOpened(statusLabel);

    QPushButton *quitButton = new QPushButton(tr("Quit"));
    connections->setReadOnly(true);

    connect(quitButton, SIGNAL(clicked(bool)),
            SLOT(close())
            );
    connect(tcpServer, SIGNAL(newConnection()),
            SLOT(new_user())
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

void EchoServer::sessionOpened(QLabel *statusLabel)
{
    tcpServer = new QTcpServer(this);
    if (!tcpServer->listen(QHostAddress::Any, PORT)) {
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

void EchoServer::new_user()
{
    QTcpSocket *clientConnection = tcpServer->nextPendingConnection();
    ServerThread *thread = new ServerThread(clientConnection->socketDescriptor(),
                                            database,
                                            this);
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(clientConnection, SIGNAL(disconnected()),
            clientConnection, SLOT(deleteLater())
            );
    connect(thread, SIGNAL(connectedUser(QString, QTcpSocket*)),
            SLOT(addNewUserToMap(QString, QTcpSocket*))
            );
    connect(thread, SIGNAL(removeUser(QString)),
            SLOT(removeUserFromMap(QString))
            );
    connect(thread, SIGNAL(addMessage(QString,QString)),
            SLOT(sendMessage(QString,QString))
            );
    thread->start();
}

void EchoServer::addNewUserToMap(QString name, QTcpSocket* tcpSocket)
{
    m_userSocket[name] = tcpSocket;
    connections->append(name);
}

void EchoServer::removeUserFromMap(QString name)
{
    m_userSocket.remove(name);
    qDebug() << name << " removed";
}

void EchoServer::sendMessage(QString name, QString message)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);
    out << PROTOCOL::ADD_MESSAGE
        << 0
        << name
        << message;

    for (auto it = m_userSocket.cbegin();
         it != m_userSocket.cend(); ++it)
    {
        qDebug() << "Sever: send message";
        it.value()->write(block);
    }

}
