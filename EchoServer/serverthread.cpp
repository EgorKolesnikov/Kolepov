#include "serverthread.h"

ServerThread::ServerThread(int socketDescriptor, SqlWrapper *db,  QObject *parent)
    : QThread(parent)
    , m_socketDescriptor(socketDescriptor)
    , database(db)
{

}

void ServerThread::run()
{
    QTcpSocket tcpSocket;

    if (!tcpSocket.setSocketDescriptor(m_socketDescriptor)) {
<<<<<<< HEAD
        emit error(tcpSocket.error());
=======
        qDebug() << "Error creating QTcpSocket in thread.\n";
>>>>>>> 29371bcbcf4af874417fa6de7b8479337cf1b4cc
        return;
    }


    tcpSocket.waitForReadyRead();

    QDataStream in(&tcpSocket);
    in.setVersion(QDataStream::Qt_4_0);

    QString user;
    in >> user;

    qDebug() << (tr("Connected %1. Address %2")
                        .arg(user)
                        .arg(tcpSocket.peerAddress().toString())
                        );

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

<<<<<<< HEAD
    bool validUser;

    if (m_users.contains(user))
    {
        out << tr("Ok!");
        validUser = true;
    }
    else
    {
        out << tr("Nope!");
        validUser = false;
    }
=======
    QSqlQuery qwe = database->get_user(user);
    if (qwe.isSelect())
    {
        qwe.next();
        if(qwe.value(1).toString().compare(user) != 0){
            out << tr("Nope!");
            tcpSocket.write(block);
            tcpSocket.waitForBytesWritten();
        }
        else{
            out << tr("Ok!");
            tcpSocket.write(block);
            tcpSocket.waitForBytesWritten();
>>>>>>> 29371bcbcf4af874417fa6de7b8479337cf1b4cc

            while(!tcpSocket.waitForReadyRead()){
                connect(tcpSocket,
                        SIGNAL(readyRead()),
                        this,
                        SLOT(manage_user_query()));
            }

        }
    }

    tcpSocket.disconnectFromHost();
    tcpSocket.waitForDisconnected();
}


void SqlWrapper::manage_user_query(){
    QTcpSocket* clientSocket = (QTcpSocket*)sender();
    int socket_id=clientSocket->socketDescriptor();


    // write answer to client
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    clientSocket->write(block);
    clientSocket->waitForBytesWritten();
}
