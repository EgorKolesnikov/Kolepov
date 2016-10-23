#include "serverthread.h"
#include "protocol.h"

ServerThread::ServerThread(int socketDescriptor, SqlWrapper *db,  QObject *parent)
    : QThread(parent)
    , m_socketDescriptor(socketDescriptor)
    , database(db)
    , m_clientDisconnected(false)
    , m_tcpSocket(new SecureSocket)
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
        m_tcpSocket->write(block);
        m_tcpSocket->flush();
    }

    m_tcpSocket->waitForReadyRead();
    m_tcpSocket->readAll();    //take from socket PROTOCOL::INIT_MESSAGE_LIST
    //If user is an admin - send him list of users
    //QChar role = authorize(user);
    QChar role = 'a';
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

        m_tcpSocket->write(block);
        m_tcpSocket->flush();
    }
    else if (role == 'm')
    {
        block.clear();
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_0);

        out << PROTOCOL::INIT_MODERATOR;
        m_tcpSocket->write(block);
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

            manageUserQuery(in, m_username, m_userID);
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

    m_tcpSocket->waitForReadyRead();
    auto clientAnswer = m_tcpSocket->readBlock();

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
    FileSource file("server_sk", true, new Base64Decoder);
    RSA::PrivateKey serverSK;
    serverSK.BERDecode(file);

    //decrypt session key
    QDataStream in(m_tcpSocket);
    in.setVersion(QDataStream::Qt_4_0);
    QByteArray data;
    in >> data;

    std::string encSessionKey(data.constData(), data.length());
    SecByteBlock sessionKey(AES::MAX_KEYLENGTH);
    RSAES_OAEP_SHA_Decryptor d(serverSK);
    StringSource (encSessionKey, true,
                 new PK_DecryptorFilter(rng, d,
                        new ArraySink(sessionKey, sessionKey.size())
                 )
    );

    in >> m_username;

    m_tcpSocket->startEncryptedMode(sessionKey);

    QSqlQuery query = database->get_user(m_username);
    if (query.isActive()) {
        query.next();
        if(query.value(1).toString().compare(m_username) == 0)
        {
            m_userID = query.value(0).toInt();
            int res = challenge(query.value(3).toByteArray());
            return res;
        }
    }
    return USER_NOT_FOUND;
}

QChar ServerThread::authorize(const QString &user_name)
{
    QSqlQuery query = database->get_user(user_name);
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

void ServerThread::manageUserQuery(QDataStream &in, const QString& user_name, int user_id){
    QString request;
    QChar ind;

    in >> ind;

    if(ind == PROTOCOL::ADD_MESSAGE){
        in >> request;
        qDebug() << user_name << request;
        emit addMessage(user_name, user_id, request);

    } else if (ind == PROTOCOL::DELETE_MESSAGE){
        QChar role = authorize(user_name);
        if (role != 'a' && role != 'm')
        {
            return;
        }
        int message_id;
        in >> message_id;
        emit deleteMessage(user_name, message_id);

    } else if (ind == PROTOCOL::MODIFY_MESSAGE){
        QChar role = authorize(user_name);
        if (role != 'a' && role != 'm')
        {
            return;
        }
        int message_id;
        in >> message_id >> request;
        emit modifyMessage(user_name, message_id, request);

    } else if (ind == PROTOCOL::SET_NEW_MODERATOR){
        QChar role = authorize(user_name);
        if (role != 'a')
        {
            return;
        }
        in >> request;
        emit changeUserRole(user_name, request, QString("m"));

    } else if (ind == PROTOCOL::DELETE_MODERATOR){
        QChar role = authorize(user_name);
        if (role != 'a')
        {
            return;
        }
        in >> request;
        emit changeUserRole(user_name, request, QString("u"));

    }
}

void ServerThread::disconnectedUser()
{
    m_clientDisconnected = true;
}
