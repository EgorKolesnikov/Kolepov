#ifndef SERVER_H
#define SERVER_H

#include <QWidget>
#include <QMap>
#include <QTcpSocket>
#include "sqlwrapper.h"

class QLabel;
class QPushButton;
class QTcpServer;
class QTextEdit;


class EchoServer : public QWidget
{
    Q_OBJECT

public:
    explicit EchoServer(QWidget *parent = Q_NULLPTR);

private slots:
    void sessionOpened(QLabel *statusLabel);
    void new_user();
    void addNewUserToMap(QString name, QTcpSocket* tcpSocket);
    void removeUserFromMap(QString name);

private:
    const qint16 PORT = 55555;

    QMap<QString, QTcpSocket*> m_userSocket;

    QTcpServer *tcpServer;
    QTextEdit *connections;

    SqlWrapper *database;
};

#endif
