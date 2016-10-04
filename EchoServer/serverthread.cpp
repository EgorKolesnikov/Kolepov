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
    qDebug() << "!!!" << database;
    QTcpSocket tcpSocket;
    if (!tcpSocket.setSocketDescriptor(m_socketDescriptor)) {
        qDebug() << "Error creating QTcpSocket in thread.\n";
        emit error(tcpSocket.error());
        return;
    }
    tcpSocket.waitForReadyRead();

    QString user;
    QDataStream in(&tcpSocket);
    in.setVersion(QDataStream::Qt_4_0);
    in >> user;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    bool authPassed = false;
    int user_id = authenticate(user);

    if(user_id == -1){
        out << PROTOCOL::LOGIN_DENIED
            << tr("Error. Maybe you should change path to database.");
    }
    else if(user_id == 0){
        out << PROTOCOL::LOGIN_DENIED << tr("Nope!");
    }
    else{
        out << PROTOCOL::LOGIN_OK << tr("Ok!");
        authPassed = true;
    }

    tcpSocket.write(block);
    tcpSocket.flush();

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
    if (qwe.isSelect()) {
        while(qwe.next()) {
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_0);
            block.clear();
            out << PROTOCOL::ADD_MESSAGE
                << qwe.value("message_id").toInt()
                << qwe.value("name").toString()
                << qwe.value("text").toString();

            tcpSocket.write(block);
            tcpSocket.flush();
        }
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

int ServerThread::authenticate(const QString &user_name){
    qDebug() << user_name;
    QSqlQuery query = database->get_user(user_name);
    qDebug() << query.isActive();
    if (query.isActive()) {
        query.next();
        if(query.value(1).toString().compare(user_name) == 0){
            qDebug() << query.value(0).toInt();
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

void ServerThread::manageUserQuery(QDataStream &in, const QString& user_name, int user_id){
    QString request;
    QChar ind;

    in >> ind;

    if(ind == PROTOCOL::ADD_MESSAGE){
        in >> request;
        qDebug() << "ServerThread: got new message";
        emit addMessage(user_name, user_id, request);
    } else if (ind == PROTOCOL::DELETE_MESSAGE){
        int message_id;
        in >> message_id;
        qDebug() << "ServerThread: delete message request.";
        emit deleteMessage(user_name, message_id);
    } else if (ind == PROTOCOL::MODIFY_MESSAGE){
        int message_id;
        in >> message_id >> request;
        qDebug() << "ServerThread: change message request.";
        emit modifyMessage(user_name, message_id, request);
    } else if (ind == PROTOCOL::SET_NEW_MODERATOR){
        in >> request;
        qDebug() << "ServerThread: set new moderator.";
        emit changeUserRole(user_name, request, QChar('u'));
    } else if (ind == PROTOCOL::DELETE_MODERATOR){
        in >> request;
        qDebug() << "ServerThread: delete moderator.";
        emit changeUserRole(user_name, request, QChar('m'));
    }
}

void ServerThread::disconnectedUser()
{
    m_clientDisconnected = true;
}
