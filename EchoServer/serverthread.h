#ifndef SERVERTHREAD_H
#define SERVERTHREAD_H

#include <string>

#include <QThread>
#include <QTcpSocket>
#include <QDataStream>
#include <QHostAddress>
#include "sqlwrapper.h"

#include "secblock.h"
#include "files.h"
#include "rsa.h"
#include "base64.h"
#include "osrng.h"
#include "secblock.h"

#include "securesocket.h"
#include "cryption.h"

using namespace CryptoPP;

class ServerThread: public QThread
{
    Q_OBJECT


public:
    ServerThread(int socketDescriptor, SqlWrapper *users, QObject *parent, QString &path_to_key);

    void run() Q_DECL_OVERRIDE;
    int authenticate();
    QChar authorize();
    int challenge(const QByteArray &clientPK);
    void manageUserQuery();


signals:
    void error(QTcpSocket::SocketError socketError);
    void connectedUser(QString name, SecureSocket* socket);
    void userAuthenticationFailed(QString name);
    void removeUser(QString name);

    void addMessage(const QString& name, int user_id, QString message);
    void deleteMessage(const QString& name, int message_id);
    void modifyMessage(const QString& name, int message_id, const QString& new_messate_text);
    void changeUserRole(const QString& who_changing, const QString& change_him, QString new_user_role);
    void getUserPasswordHalf(const QString& username);

public slots:
    void disconnectedUser();

private:
    int m_socketDescriptor;
    bool m_clientDisconnected;
    SqlWrapper *database;
    SecureSocket *m_tcpSocket;

    QString m_username;
    QString path_to_key;
    int m_userID;

    static const int USER_NOT_FOUND = -1;
    static const int CHALLENGE_PASSED = 0;
    static const int CHALLENGE_FAILED = 1;
    static const int INCORRECT_CERTIFACATE = 2;

};

#endif // SERVERTHREAD_H
