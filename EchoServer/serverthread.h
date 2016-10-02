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

private:
<<<<<<< HEAD
    QTcpSocket *m_tcpSocket;
    QString m_username;
   // int
    const QStringList& m_users;


    bool authentication();
=======
    int m_socketDescriptor;
    SqlWrapper *database;
>>>>>>> 29371bcbcf4af874417fa6de7b8479337cf1b4cc
};

#endif // SERVERTHREAD_H
