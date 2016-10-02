#include "serverthread.h"
#include "protocol.h"

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
        qDebug() << "Error creating QTcpSocket in thread.\n";
        emit error(tcpSocket.error());
        return;
    }
    qDebug() << m_socketDescriptor;

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

    //TODO: Make authentication function, which returns true or false
    // I know it's bad. It was too late.
    QSqlQuery qwe = database->get_user(user);
    if (qwe.isSelect())
    {
        qwe.next();
        if(qwe.value(1).toString().compare(user) != 0){
            out << PROTOCOL::LOGIN_DENIED  << tr("Nope!");
        }
        else{
            out << PROTOCOL::LOGIN_OK << tr("Ok!");
        }
    }
    else
    {
        out << PROTOCOL::LOGIN_DENIED  << tr("qwe.isSelect == false");
    }

    tcpSocket.write(block);
    tcpSocket.waitForBytesWritten();


    tcpSocket.disconnectFromHost();
    tcpSocket.waitForDisconnected();
}
