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
    int authenticate(const QString& user_name);
    QChar authorize(const QString& user_name);
    void manageUserQuery(QDataStream& in, const QString& user_name, int user_id);


signals:
    void error(QTcpSocket::SocketError socketError);
    void connectedUser(QString name, QTcpSocket* socket);
    void removeUser(QString name);

    void addMessage(const QString& name, int user_id, QString message);
    void deleteMessage(const QString& name, int message_id);
    void modifyMessage(const QString& name, int message_id, const QString& new_messate_text);
    void changeUserRole(const QString& who_changing, const QString& change_him, QString new_user_role);

private slots:
    void disconnectedUser();

private:
    int m_socketDescriptor;
    bool m_clientDisconnected;
    SqlWrapper *database;
};

#endif // SERVERTHREAD_H
