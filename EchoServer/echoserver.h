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
    void sessionOpened();
    void replyToClient();

private:
    QLabel *statusLabel;
    QTcpServer *tcpServer;
    QTextEdit *connections;
    QStringList users;
    SqlWrapper *database;
};

#endif
