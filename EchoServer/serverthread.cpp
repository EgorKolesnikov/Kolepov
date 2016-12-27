#include "serverthread.h"
#include "protocol.h"

ServerThread::ServerThread(int socketDescriptor, SqlWrapper *db,  QObject *parent, QString& key_path)
    : QThread(parent)
    , m_socketDescriptor(socketDescriptor)
    , database(db)
    , m_clientDisconnected(false)
    , m_tcpSocket(new SecureSocket)
    , path_to_key(key_path)
{

}

void ServerThread::run()
{
    if (!m_tcpSocket->setSocketDescriptor(m_socketDescriptor)) {
        qDebug() << "Error creating QTcpSocket in thread.\n";
        emit error(m_tcpSocket->error());
        return;
    }
    m_tcpSocket->waitForReadyRead();

    int authRes = authenticate();

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);


    switch (authRes)
    {
        case USER_NOT_FOUND:
        case CHALLENGE_FAILED:
            out << PROTOCOL::LOGIN_DENIED << tr("Check authentication data.");
            break;

        case CHALLENGE_PASSED:
            out << PROTOCOL::LOGIN_OK;
            break;

        case INCORRECT_CERTIFACATE:
            return;
    }

    m_tcpSocket->writeBlock(block);
    m_tcpSocket->flush();
    m_tcpSocket->waitForBytesWritten();

    if (authRes != CHALLENGE_PASSED)
    {
        m_tcpSocket->disconnectFromHost();
        m_tcpSocket->waitForDisconnected();
        emit userAuthenticationFailed(m_username);
        return;
    }

    emit connectedUser(m_username, m_tcpSocket);

    connect(m_tcpSocket, SIGNAL(disconnected()),
            SLOT(disconnectedUser()));


    //Sending all messages from database
    QSqlQuery qwe = database->get_all_messages();
    if (qwe.isActive()) {
        qint32 numOfInitMessages = 0;
        block.clear();
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_0);
        out << PROTOCOL::SEND_INIT_MESSAGES
            << numOfInitMessages;
        while(qwe.next()) {
            ++numOfInitMessages;
            out << qwe.value("message_id").toInt()
                << qwe.value("name").toString()
                << qwe.value("text").toString();           
        }
        out.device()->seek(sizeof(PROTOCOL::SEND_INIT_MESSAGES));
        out << numOfInitMessages;
        m_tcpSocket->writeBlock(block);
        m_tcpSocket->flush();
    }

    m_tcpSocket->waitForReadyRead();
    m_tcpSocket->readAll();    //take from socket PROTOCOL::INIT_MESSAGE_LIST
    //If user is an admin - send him list of users
    QChar role = authorize();

    if (role == 'a')
    {
        block.clear();
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_0);

        out << PROTOCOL::USER_MODERATOR_LIST
            << qint32(0);

        qint32 membersCount = 0;
        QSqlQuery query = database->get_all_users();
        while (query.next())
        {
            ++membersCount;
            out << query.value("name").toString()
                << query.value("role").toString();
        }
        out.device()->seek(sizeof(PROTOCOL::USER_MODERATOR_LIST));
        out << membersCount;

        m_tcpSocket->writeBlock(block);
        m_tcpSocket->flush();
    }
    else if (role == 'm')
    {
        block.clear();
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_0);

        out << PROTOCOL::INIT_MODERATOR;
        m_tcpSocket->writeBlock(block);
        m_tcpSocket->flush();
    }

    //Main cycle
    while(!m_clientDisconnected)
    {
        if (m_tcpSocket->waitForReadyRead())
        {
            QDataStream in(m_tcpSocket);
            in.setVersion(QDataStream::Qt_4_0);

            in.startTransaction();
            if (!in.commitTransaction())
                continue;

            manageUserQuery();
        }
    }

    emit removeUser(m_username);
}

int ServerThread::challenge(const QByteArray& clientPK)
{

    auto texts = Cryption::generateTextForChecking(
                clientPK,
                16);
    m_tcpSocket->writeBlock(texts.second);

    if (!m_tcpSocket->waitForReadyRead(10000))
        return CHALLENGE_FAILED;
    QPair<bool, QByteArray> clientAnswer;
    try
    {
        clientAnswer = m_tcpSocket->readBlock();
    }
    catch (...)
    {
        return CHALLENGE_FAILED;
    }

    if (!clientAnswer.first)
    {
        qDebug() << "Message corrupted.";
        return CHALLENGE_FAILED;
    }

    if (texts.first != clientAnswer.second)
    {
        qDebug() << "Incorrect decryption.";
        return CHALLENGE_FAILED;
    }

    return CHALLENGE_PASSED;
}

int ServerThread::authenticate()
{
    //read server_sk from file and decode from base64
    AutoSeededRandomPool rng;
    FileSource file(path_to_key.toStdString().c_str(), true, new Base64Decoder);
    RSA::PrivateKey serverSK;
    serverSK.BERDecode(file);

    //decrypt session key
    QDataStream in(m_tcpSocket);
    in.setVersion(QDataStream::Qt_4_0);
    QByteArray data;
    in >> data;

    std::string encSessionKey(data.constData(), data.length());
    SecByteBlock sessionKey(AES::MAX_KEYLENGTH);
    try
    {
    RSAES_OAEP_SHA_Decryptor d(serverSK);
    StringSource (encSessionKey, true,
                 new PK_DecryptorFilter(rng, d,
                        new ArraySink(sessionKey, sessionKey.size())
                 )
    );
    }
    catch (...)
    {
        return INCORRECT_CERTIFACATE;
    }

    in >> m_username;

    m_tcpSocket->startEncryptedMode(sessionKey);

    QSqlQuery query = database->get_user(m_username);
    if (query.isActive()) {
        query.next();
        if(query.value(1).toString().compare(m_username) == 0)
        {
            m_userID = query.value(0).toInt();
            int res = challenge(query.value(3).toByteArray());
            qDebug() << "???" << res << "\n";
            return res;
        }
    }
    qDebug() << "!!!!!!!!!!!!!!\n";
    return USER_NOT_FOUND;
}

QChar ServerThread::authorize()
{
    QSqlQuery query = database->get_user(m_username);
    if (query.isActive())
    {
        query.next();
        return query.value("role").toString()[0];
    }
    else
    {
        return 'e'; //error
    }
}

void ServerThread::manageUserQuery(){

    while (!m_tcpSocket->peek(1).isEmpty())
    {
        auto data = m_tcpSocket->readBlock();
        QDataStream in(data.second);
        in.setVersion(QDataStream::Qt_4_0);

        QString request;
        QChar ind;

        in >> ind;

        if(ind == PROTOCOL::ADD_MESSAGE){
            in >> request;
            emit addMessage(m_username, m_userID, request);

        } else if (ind == PROTOCOL::DELETE_MESSAGE){
            QChar role = authorize();
            if (role != 'a' && role != 'm')
            {
                return;
            }
            int message_id;
            in >> message_id;
            emit deleteMessage(m_username, message_id);

        } else if (ind == PROTOCOL::MODIFY_MESSAGE){
            QChar role = authorize();
            if (role != 'a' && role != 'm')
            {
                return;
            }
            int message_id;
            in >> message_id >> request;
            emit modifyMessage(m_username, message_id, request);

        } else if (ind == PROTOCOL::SET_NEW_MODERATOR){
            QChar role = authorize();
            if (role != 'a')
            {
                return;
            }
            in >> request;
            emit changeUserRole(m_username, request, QString("m"));

        } else if (ind == PROTOCOL::DELETE_MODERATOR){
            QChar role = authorize();
            if (role != 'a')
            {
                return;
            }
            in >> request;
            emit changeUserRole(m_username, request, QString("u"));
        } else if(ind == PROTOCOL::USER_GET_PASSWORD_HALF){
            emit getUserPasswordHalf(m_username);
        }

       }
}

void ServerThread::disconnectedUser()
{
    qDebug() << "DDDDDIIIIIIIIIIIIIIIIIIIIIIIIIISSSSCONECTED!!!";
    m_tcpSocket->disconnectFromHost();
    m_tcpSocket->waitForDisconnected();
    m_clientDisconnected = true;
}
