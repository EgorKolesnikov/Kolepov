#ifndef CLIENT_H
#define CLIENT_H

#include <QDialog>
#include <QTcpSocket>
#include <QLineEdit>
#include "protocol.h"

class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTcpSocket;
class QProgressBar;

class ConnectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectDialog(QWidget *parent = Q_NULLPTR);

    QTcpSocket* socket() { return tcpSocket; }
    QString username() { return loginLineEdit->text(); }

private slots:
    void connectToServer();
    void displayError(QAbstractSocket::SocketError socketError);
    void enableConnectButton();
    void sendLogin();
    void showResult();

private:
    const quint16 PORT = 55555;

    QComboBox *hostCombo;
    QLineEdit *portLineEdit;
    QLineEdit *loginLineEdit;
    QPushButton *connectButton;
    QProgressBar *connectProgress;

    QTcpSocket *tcpSocket;
};

#endif
