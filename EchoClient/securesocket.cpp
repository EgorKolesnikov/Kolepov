#include "securesocket.h"

SecureSocket::SecureSocket(QObject *parent)
    : QTcpSocket(parent)
    , m_useEncryption(false)
{}

void SecureSocket::startEncryptedMode(const CryptoPP::SecByteBlock &key)
{
    m_key = key;
    m_useEncryption = true;
}

bool SecureSocket::isEncryptedEnable() const
{
    return m_useEncryption;
}

qint64 SecureSocket::writeBlock(const QByteArray &message)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    if (m_useEncryption)
    {
        auto pair = Cryption::encryptMessage(m_key, message);
        out << pair;
    }
    else
    {
        out << message;
    }
    return QTcpSocket::write(block);
}

QPair<bool, QByteArray> SecureSocket::readBlock()
{
    QDataStream in(this);
    in.setVersion(QDataStream::Qt_4_0);

    if (m_useEncryption)
    {
        QPair<QByteArray, QByteArray> pair;
        QPair<bool, QByteArray> res;
        in >> pair;
        try
        {
            res = Cryption::decryptMessage(m_key, pair);
        }
        catch (...)
        {
            res = qMakePair(false, Q_NULLPTR);
        }
        return res;
    }
    else
    {
        QByteArray block;
        in >> block;
        return qMakePair(true, block);
    }
}
