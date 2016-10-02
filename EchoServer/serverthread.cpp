#include "serverthread.h"

ServerThread::ServerThread(int socketDescriptor, const QStringList& users,  QObject *parent)
    : QThread(parent)
    , m_socketDescriptor(socketDescriptor)
    , m_users(users)
{

}

void ServerThread::run()
{
    QTcpSocket tcpSocket;

    if (!tcpSocket.setSocketDescriptor(m_socketDescriptor)) {
     //   emit error(tcpSocket.error());
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

    if (m_users.contains(user))
    {
        out << tr("Ok!");
    }
    else
    {
        out << tr("Nope!");
    }

    tcpSocket.write(block);
    tcpSocket.waitForBytesWritten();

    tcpSocket.disconnectFromHost();
    tcpSocket.waitForDisconnected();


}
