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

    void show() /*Q_DECL_OVERRIDE*/;

private slots:


private:
    QTabWidget *m_tabWidget;

    QTableWidget *m_messages;
    QLineEdit *m_inputMessageEdit;
    QPushButton *m_sendButton;

    QPushButton *m_modifyButton;
    QPushButton *m_deleteButton;

    QTcpSocket *m_tcpSocket;
};

#endif // ECHOCLIENT_H
