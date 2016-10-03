#ifndef SERVERTHREAD_H
#define SERVERTHREAD_H

#include <QThread>
#include <QTcpSocket>
#include <QDataStream>
#include <QHostAddress>
#include "sqlwrapper.h"


class ServerThread: public QThread
{
    Q_OBJECT

public:
    ServerThread(int socketDescriptor, SqlWrapper *users, QObject *parent);

    void run() Q_DECL_OVERRIDE;


signals:
    void error(QTcpSocket::SocketError socketError);
    void connectedUser(QString info);


private:
    int m_socketDescriptor;
    SqlWrapper *database;
};

#endif // SERVERTHREAD_H
