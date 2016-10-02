#ifndef SERVERTHREAD_H
#define SERVERTHREAD_H

#include <QThread>
#include <QTcpSocket>
#include <QDataStream>
#include <QHostAddress>


class ServerThread: public QThread
{
    Q_OBJECT

public:
    ServerThread(int socketDescriptor, const QStringList& users, QObject *parent);

    void run() Q_DECL_OVERRIDE;

signals:
    void error(QTcpSocket::SocketError socketError);

private:
    QTcpSocket *m_tcpSocket;
    QString m_username;
   // int
    const QStringList& m_users;


    bool authentication();
};

#endif // SERVERTHREAD_H
