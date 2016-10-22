#include "serverthread.h"
#include "protocol.h"

ServerThread::ServerThread(int socketDescriptor, SqlWrapper *db,  QObject *parent)
    : QThread(parent)
    , m_socketDescriptor(socketDescriptor)
    , database(db)
    , m_clientDisconnected(false)
{

}

void ServerThread::run()
{
    SecureSocket tcpSocket;
    if (!tcpSocket.setSocketDescriptor(m_socketDescriptor)) {
        qDebug() << "Error creating QTcpSocket in thread.\n";
        emit error(tcpSocket.error());
        return;
    }
    tcpSocket.waitForReadyRead();

    authenticate(&tcpSocket);
    QString user = "Test";

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    bool authPassed = false;
    //int user_id = authenticate(user);
    int user_id = 0;

    if(user_id == -1){
        out << PROTOCOL::LOGIN_DENIED
            << tr("Error. Maybe you should change path to database.");
    }
    else if(user_id == 0){
        out << PROTOCOL::LOGIN_DENIED << tr("Access denied.");
    }
    else{
        out << PROTOCOL::LOGIN_OK;
        authPassed = true;
    }

    tcpSocket.write(block);
    tcpSocket.flush();
    tcpSocket.waitForBytesWritten();

    if (!authPassed)
    {
        tcpSocket.disconnectFromHost();
        tcpSocket.waitForDisconnected();
        return;
    }

    emit connectedUser(user, &tcpSocket);

    connect(&tcpSocket, SIGNAL(disconnected()),
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
        tcpSocket.write(block);
        tcpSocket.flush();
    }

    tcpSocket.waitForReadyRead();
    tcpSocket.readAll();    //take from socket PROTOCOL::INIT_MESSAGE_LIST
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

        tcpSocket.write(block);
        tcpSocket.flush();
    }
    else if (role == 'm')
    {
        block.clear();
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_0);

        out << PROTOCOL::INIT_MODERATOR;
        tcpSocket.write(block);
        tcpSocket.flush();
    }

    //Main cycle
    while(!m_clientDisconnected)
    {
        if (tcpSocket.waitForReadyRead())
        {
            QDataStream in(&tcpSocket);
            in.setVersion(QDataStream::Qt_4_0);

            in.startTransaction();
            if (!in.commitTransaction())
                continue;

            manageUserQuery(in, user, user_id);
        }
    }
    emit removeUser(user);
}

int ServerThread::authenticate(SecureSocket *tcpSocket)
{
    //read server_sk from file and decode from base64
    AutoSeededRandomPool rng;
    FileSource file("server_sk", true, new Base64Decoder);
    RSA::PrivateKey serverSK;
    serverSK.BERDecode(file);

    //decrypt session key
    QDataStream in(tcpSocket);
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

    QString user;
    in >> user;

    tcpSocket->startEncryptedMode(sessionKey);

    QSqlQuery query = database->get_user(user);
    if (query.isActive()) {
        query.next();
        if(query.value(1).toString().compare(user) == 0)
        {
            auto texts = Cryption::generateTextForChecking(
                        query.value(3).toByteArray(),
                        16);
            tcpSocket->writeBlock(texts.second);

            if (!tcpSocket->waitForReadyRead())
            {
                qDebug() << "Auth " << user << " failed.";
                return 0;
            }

            auto clientAnswer = tcpSocket->readBlock();
            if (!clientAnswer.first)
            {
                qDebug() << "Message corrupted.";
                return 0;
            }

            if (texts.first != clientAnswer.second)
            {
                qDebug() << "Incorrect dectyprion.";
                return 0;
            }

            qDebug() << "passed";

            return query.value(0).toInt();
        }
        else{
            return 0;
        }
    }
    else {
        return -1;
    }
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
