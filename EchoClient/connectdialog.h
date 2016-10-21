#ifndef CLIENT_H
#define CLIENT_H

#include <QDialog>
#include <QTcpSocket>
#include <QLineEdit>
#include "protocol.h"

#include <string>

#include "sha.h"
#include "secblock.h"
#include "files.h"
#include "base64.h"
#include "rsa.h"
#include "aes.h"
#include "osrng.h"

class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTcpSocket;
class QProgressBar;

using namespace CryptoPP;

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
    void browseKeyFile();

private:
    const quint16 PORT = 55555;

    QComboBox *hostCombo;
    QLineEdit *portLineEdit;
    QLineEdit *loginLineEdit;
    QLineEdit *passwordLineEdit;
    QLineEdit *keyPathLineEdit;
    QPushButton *connectButton;
    QProgressBar *connectProgress;

    QTcpSocket *tcpSocket;

    SecByteBlock getHashFromPassword(const QString& password);
    void makeSecureConnection();

};

#endif
