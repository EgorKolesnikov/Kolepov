#include <QApplication>
#include <QtCore>
#include <stdlib.h>
#include "echoserver.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QGuiApplication::setApplicationDisplayName(
                EchoServer::tr("Server"));

    EchoServer server;
    server.show();
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

    return app.exec();

//    SqlWrapper::create_database();
//    return 0;
}
