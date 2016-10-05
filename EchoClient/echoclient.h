#ifndef ECHOCLIENT_H
#define ECHOCLIENT_H

#include <QtWidgets>
#include <QTcpSocket>
#include <QMap>

#include "connectdialog.h"

class EchoClient : public QWidget
{
    Q_OBJECT

public:
    EchoClient(QWidget *parent = 0);
    ~EchoClient();

public slots:
    void enableSendButton();
    void sendMessage();
    void show();
    void readServerResponse();
    void serverDisconected();
    void deleteMessageRequest();

private:
    void addMessage(int messageId, const QString& user, const QString& text);
    void deleteMessageResponse(int messageId);

private:
    QTabWidget *m_tabWidget;
    QMap<int, int> m_messageIdRowNum;

    QTableWidget *m_messages;
    QLineEdit *m_inputMessageEdit;
    QPushButton *m_sendButton;

    QPushButton *m_modifyButton;
    QPushButton *m_deleteButton;

    QListWidget *m_userList;
    QListWidget *m_moderatorList;

    QTcpSocket *m_tcpSocket;
};

#endif // ECHOCLIENT_H
