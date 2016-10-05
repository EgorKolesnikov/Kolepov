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
    void dialogResult(int code);
    void enableSendButton();
    void sendMessage();
    void show();
    void readServerResponse();
    void serverDisconected();
    void deleteMessageRequest();
    void modifyRequest();
    //for admin only
    void addModeratorRequest();
    void deleteModeratorRequest();

private:
    void addMessage(int messageId, const QString& user, const QString& text);
    void deleteMessageResponse(int messageId);
    void modifyResponse(int messageId, QString text);
    void enableAdminMode();
    void enableModeratorMode();
    void disableModeratorMode();

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

    QWidget* m_adminWidget;

    QTcpSocket *m_tcpSocket;
};

#endif // ECHOCLIENT_H
