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
    QTcpSocket tcpSocket;

    if (!tcpSocket.setSocketDescriptor(m_socketDescriptor)) {
        qDebug() << "Error creating QTcpSocket in thread.\n";
        emit error(tcpSocket.error());
        return;
    }

    tcpSocket.waitForReadyRead();

    QDataStream in(&tcpSocket);
    in.setVersion(QDataStream::Qt_4_0);

    QString user;
    in >> user;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    //TODO: Make authentication function, which returns true or false
    // I know it's bad. It was too late.
    bool authPassed = false;
    QSqlQuery qwe = database->get_user(user);
    if (qwe.isSelect())
    {
        qwe.next();
        if(qwe.value(1).toString().compare(user) != 0){
            out << PROTOCOL::LOGIN_DENIED  << tr("Nope!");
        }
        else{
            out << PROTOCOL::LOGIN_OK << tr("Ok!");
            authPassed = true;
        }
    }
    else
    {
        out << PROTOCOL::LOGIN_DENIED  << tr("qwe.isSelect == false");
    }

    tcpSocket.write(block);
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
    qwe = database->get_all_messages();
    if (qwe.isSelect())
    {
        while(qwe.next())
        {
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_0);
            block.clear();
            out << PROTOCOL::ADD_MESSAGE
                << qwe.value("message_id").toInt()
                << qwe.value("name").toString()
                << qwe.value("text").toString();

            tcpSocket.write(block);
        }

    }

    //Main cycle
    while(!m_clientDisconnected)
    {
        if (tcpSocket.waitForReadyRead())
        {
            QString request;
            QChar ind;
            QDataStream in(&tcpSocket);
            in.setVersion(QDataStream::Qt_4_0);

//            in.startTransaction();
//            if (!in.commitTransaction())
//                continue;
            in >> ind;
            if  (ind == PROTOCOL::ADD_MESSAGE)
            {
                in >> request;
                emit addMessage(request);
            }
        }
    }
    emit removeUser(user);
}

void ServerThread::disconnectedUser()
{
    m_clientDisconnected = true;
}
