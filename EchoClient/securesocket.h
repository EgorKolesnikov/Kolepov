#ifndef SECURESOCKET_H
#define SECURESOCKET_H

#include <QTcpSocket>
#include <QSet>
#include "cryption.h"

class SecureSocket : public QTcpSocket
{
    Q_OBJECT

public:
    SecureSocket(QObject *parent=Q_NULLPTR);

    void startEncryptedMode(const SecByteBlock& key);
    bool isEncryptedEnable() const;
    qint64 writeBlock(const QByteArray& message);
    QPair<bool, QByteArray> readBlock();

private:
    bool m_useEncryption;
    SecByteBlock m_key;
    QSet<QByteArray> m_usedIV;
};

#endif // SECURESOCKET_H
