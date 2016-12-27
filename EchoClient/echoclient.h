#ifndef ECHOCLIENT_H
#define ECHOCLIENT_H

#include <QtWidgets>
#include <QTcpSocket>
#include <QMap>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QMutex>
#include <QMutexLocker>
#include <stdlib.h>
#include "pwdbased.h"
#include "sha.h"
#include "hkdf.h"

#include "connectdialog.h"
#include "securesocket.h"

class QSqlDatabase;
class QSqlQuery;
class QSqlError;
class QSqlRecord;
class QMutex;
class QMutexLocker;

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

    //for notes
    void addNoteRequest();
    void deleteNoteRequest();
    void modifyNoteRequest();
    void refreshNotesRequest();
    void tabBarTabChanged(int index);


private:
    void addMessage(int messageId, const QString& user, const QString& text);
    void deleteMessageResponse(int messageId);
    void modifyResponse(int messageId, QString text);
    void enableAdminMode();
    void enableModeratorMode();
    void disableModeratorMode();
    int get_available_id();
    void connect_to_client_database(QString &server_password_half);
    QString kdf(QString& password, QString& salt);

private:
    QTabWidget *m_tabWidget;

    QTableWidget *m_messages;
    QMap<int, int> m_messageIdRowNum;
    QLineEdit *m_inputMessageEdit;
    QPushButton *m_sendButton;

    QWidget* m_adminWidget;
    QPushButton *m_modifyButton;
    QPushButton *m_deleteButton;
    QListWidget *m_userList;
    QListWidget *m_moderatorList;

    QWidget* m_notesWidget;
    QTableWidget* m_notes;
    QLineEdit *m_inputNoteEdit;
    QPushButton *m_addNoteButton;
    QPushButton *m_deleteNoteButton;
    QPushButton *m_modifyNoteButton;
    QPushButton *m_refreshNotesButton;

    SecureSocket *m_tcpSocket;
    QSqlDatabase db_connection_;
    QString database_password;
    QString username;
};

#endif // ECHOCLIENT_H
