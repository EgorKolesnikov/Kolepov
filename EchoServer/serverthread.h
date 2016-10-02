#ifndef SERVERTHREAD_H
#define SERVERTHREAD_H

#include <QThread>
#include <QTcpSocket>
#include <QDataStream>
#include <QHostAddress>


class ServerThread: public QThread
{
public:
    ServerThread(int socketDescriptor, const QStringList& users, QObject *parent);

    void run() Q_DECL_OVERRIDE;

//signals:
//    void error(QTcpSocket::SocketError socketError);

private:
    int m_socketDescriptor;
    const QStringList& m_users;
};

#endif // SERVERTHREAD_H
