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

void EchoServer::show(){
    InsertKeyDialog *dialog = new InsertKeyDialog(this);
    connect(dialog, SIGNAL(finished(int)), this, SLOT(dialogResult(int)),
            Qt::QueuedConnection);
    dialog->exec();
    dialog->deleteLater();
}

void EchoServer::dialogResult(int code){
    InsertKeyDialog *dialog = (InsertKeyDialog*)QObject::sender();
    if(dialog->result() == QDialog::Accepted){
        server_sk_file_path = dialog->getChoosenServerKeyPath();
        server_database_password = dialog->getChoosenDatabaseKeyPath();
        database = new SqlWrapper(this, server_database_password);
        QWidget::show();
    }
    else{
        QApplication::quit();
    }
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
    statusLabel->setText(tr("The server is running on\n\nIP: %1\nport: %2\n\n"
                            "Connection log:")
                         .arg(ipAddress).arg(tcpServer->serverPort()));
}

void EchoServer::new_user()
{
    QTcpSocket *clientConnection = tcpServer->nextPendingConnection();
    ServerThread *thread = new ServerThread(clientConnection->socketDescriptor(),
                                            database,
                                            this,
                                            server_sk_file_path
                                            );
    connect(thread, SIGNAL(finished()),
            thread, SLOT(deleteLater())
            );
    connect(clientConnection, SIGNAL(disconnected()),
            clientConnection, SLOT(deleteLater())
            );


    connect(thread, SIGNAL(connectedUser(QString,  SecureSocket*)),
            SLOT(addNewUserToMap(QString,  SecureSocket*))
            );
    connect(thread, SIGNAL(removeUser(QString)),
            SLOT(removeUserFromMap(QString))
            );
    connect(thread, SIGNAL(userAuthenticationFailed(QString)),
            SLOT(userAuthenticationFailed(QString))
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
    connect(thread, SIGNAL(changeUserRole(QString,QString,QString)),
            SLOT(changeUserRole(QString,QString,QString))
            );

    thread->start();
}

void EchoServer::addNewUserToMap(QString name, SecureSocket *tcpSocket)
{
    if (m_userSocket.find(name) != m_userSocket.end())
    {
        connections->append(tr("%1 | %2 forced disconnected")
                            .arg(QTime::currentTime().toString())
                            .arg(name)
                            );
        qDebug() << name;
        connect(this, SIGNAL(disconnectUser()),
                (ServerThread*)sender(), SLOT(disconnectedUser())
                );
        emit disconnectUser();
        return;

    }
    m_userSocket[name] = tcpSocket;
    connections->append(tr("%1 | %2 connected")
                        .arg(QTime::currentTime().toString())
                        .arg(name)
                        );
}

void EchoServer::userAuthenticationFailed(QString username)
{
    connections->append(tr("%1 | %2 failed authentication")
                        .arg(QTime::currentTime().toString())
                        .arg(username)
                        );
}

void EchoServer::removeUserFromMap(QString name)
{
    m_userSocket.remove(name);
    connections->append(tr("%1 | %2 disconnected")
                        .arg(QTime::currentTime().toString())
                        .arg(name)
                        );
}

void EchoServer::sendMessage(const QString& name, int user_id, const QString& message)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    if (!database->add_message(user_id, message))
    {
        qDebug() << "Server: error while additing message";
        return;
    }

    int added_message_id = database->get_message_id(user_id, message);

    out << PROTOCOL::ADD_MESSAGE
        << added_message_id
        << name
        << message;

    for (auto it = m_userSocket.cbegin();
         it != m_userSocket.cend(); ++it)
    {
        it.value()->writeBlock(block);
        it.value()->flush();
    }
}

void EchoServer::deleteMessage(const QString &name, int message_id){
    if (!database->delete_message(message_id))
    {
        qDebug() << "Server: couldn't delete message " << message_id;
        return;
    }

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    out << PROTOCOL::DELETE_MESSAGE
        << message_id;

    for (auto it = m_userSocket.cbegin();
         it != m_userSocket.cend(); ++it)
    {
        it.value()->writeBlock(block);
        it.value()->flush();
    }
}

void EchoServer::modifyMessage(const QString &name, int message_id, const QString &new_message_text){

    if (!database->modify_message(message_id, new_message_text))
    {
        qDebug() << "Server: couldn't modify message " << message_id;
        return;
    }

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    out << PROTOCOL::MODIFY_MESSAGE
        << message_id
        << new_message_text;

    for (auto it = m_userSocket.cbegin();
         it != m_userSocket.cend(); ++it)
    {
        it.value()->writeBlock(block);
        it.value()->flush();
    }
}

void EchoServer::changeUserRole(const QString &who_changing, const QString &change_him, QString new_role){

    if (!database->change_user_role(change_him, new_role))
    {
        qDebug() << "Server: couldn't change role " << change_him;
        return;
    }

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    if(new_role == "m"){
        out << PROTOCOL::SET_NEW_MODERATOR;
    }
    else{
        out << PROTOCOL::DELETE_MODERATOR;
    }

    if (m_userSocket.contains(change_him))
    {
        m_userSocket[change_him]->writeBlock(block);
        m_userSocket[change_him]->flush();
    }
}
