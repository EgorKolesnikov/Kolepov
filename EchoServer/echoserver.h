#ifndef SERVER_H
#define SERVER_H

#include <QWidget>
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
    void displayNewUser(QString username);

private:
    const qint16 PORT = 55555;

    QTcpServer *tcpServer;
    QTextEdit *connections;

    SqlWrapper *database;
};

#endif
