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
    QTcpSocket *m_tcpSocket;
    QString m_username;
    int m_socketDescriptor;
    SqlWrapper *database;

public slots:
    void manage_user_query();
};

#endif // SERVERTHREAD_H
