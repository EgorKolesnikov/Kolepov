#ifndef ECHOCLIENT_H
#define ECHOCLIENT_H

#include <QtWidgets>
#include <QTcpSocket>

#include "connectdialog.h"

class EchoClient : public QWidget
{
    Q_OBJECT

public:
    EchoClient(QWidget *parent = 0);
    ~EchoClient();

public slots:
    void sendMessage();
    void show();
    void readServerResponse();
    void serverDisconected();

private:
    void addMessage(int messageId, const QString& user, const QString& text);

private:
    QTabWidget *m_tabWidget;

    QTableWidget *m_messages;
    QLineEdit *m_inputMessageEdit;

    QPushButton *m_modifyButton;
    QPushButton *m_deleteButton;

    QListWidget *m_userList;
    QListWidget *m_moderatorList;

    QTcpSocket *m_tcpSocket;
};

#endif // ECHOCLIENT_H
