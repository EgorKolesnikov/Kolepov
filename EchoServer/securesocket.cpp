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
        AutoSeededRandomPool rng;
        byte iv[12];
        QByteArray ivInBytes;
        do
        {
            rng.GenerateBlock(iv, sizeof(iv));
            ivInBytes = QByteArray::fromRawData(
                        (char*)iv, sizeof(iv));
        }
        while (m_usedIV.contains(ivInBytes));
        m_usedIV.insert(ivInBytes);

        QByteArray cipher = Cryption::encryptMessage(
                    m_key,
                    iv, sizeof(iv),
                    message);
        out << qMakePair(ivInBytes, cipher);
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
            m_usedIV.insert(pair.first);
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
