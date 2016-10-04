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
    connect(thread, SIGNAL(finished()),
            thread, SLOT(deleteLater())
            );
    connect(clientConnection, SIGNAL(disconnected()),
            clientConnection, SLOT(deleteLater())
            );

    connect(thread, SIGNAL(connectedUser(QString, QTcpSocket*)),
            SLOT(addNewUserToMap(QString, QTcpSocket*))
            );
    connect(thread, SIGNAL(removeUser(QString)),
            SLOT(removeUserFromMap(QString))
            );

    connect(thread, SIGNAL(addMessage(QString, int, QString)),
            SLOT(sendMessage(QString, int, QString))
            );
    connect(thread, SIGNAL(deleteMessage(QString,int)),
            SLOT(deleteMessage(QString,int))
            );
    connect(thread, SIGNAL(modifyMessage(QString,int,QString)),
            SLOT(modifyMessage(QString,int,QString))
            );
    connect(thread, SIGNAL(changeUserRole(QString,QString,QChar)),
            SLOT(changeUserRole(QString,QString,QChar))
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

void EchoServer::sendMessage(const QString& name, int user_id, const QString& message)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    bool success = database->add_message(user_id, message);
    QSqlQuery added_message = database->get_message_id(message);

    out << PROTOCOL::ADD_MESSAGE
        << added_message.value(0).toInt()
        << name
        << message;

    qDebug() << "Server: " << name << " sent message.";
    qDebug() << added_message.value(0).toInt();
    for (auto it = m_userSocket.cbegin();
         it != m_userSocket.cend(); ++it)
    {
        it.value()->write(block);
        it.value()->flush();
    }
}

void EchoServer::deleteMessage(const QString &name, int message_id){
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    bool success = database->delete_message(message_id);

    out << PROTOCOL::DELETE_MESSAGE
        << message_id;

    qDebug() << "Server: " << name << " deleted message.";
    for (auto it = m_userSocket.cbegin();
         it != m_userSocket.cend(); ++it)
    {
        it.value()->write(block);
        it.value()->flush();
    }
}

void EchoServer::modifyMessage(const QString &name, int message_id, const QString &new_message_text){
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    bool success = database->modify_message(message_id, new_message_text);

    out << PROTOCOL::MODIFY_MESSAGE
        << message_id
        << new_message_text;

    qDebug() << "Server: " << name << " modified message.";
    for (auto it = m_userSocket.cbegin();
         it != m_userSocket.cend(); ++it)
    {
        it.value()->write(block);
        it.value()->flush();
    }
}

void EchoServer::changeUserRole(const QString &who_changing, const QString &change_him, QChar new_role){
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    bool success = database->change_user_role(change_him, new_role);

    if(new_role == 'm'){
        out << PROTOCOL::SET_NEW_MODERATOR
            << change_him;
    }
    else{
        out << PROTOCOL::DELETE_MODERATOR
            << change_him;
    }

    qDebug() << "Server: " << who_changing << " changed user role.";
    for (auto it = m_userSocket.cbegin();
         it != m_userSocket.cend(); ++it)
    {
        it.value()->write(block);
        it.value()->flush();
    }
}
