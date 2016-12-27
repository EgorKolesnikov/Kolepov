#include "echoclient.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

//    QSqlDatabase db_connection_ = QSqlDatabase::addDatabase("QSQLITE");
//    db_connection_.setDatabaseName("/home/kolegor/Code/Kolepov/EchoClient/db.sqlite");
//    db_connection_.open();

//    QSqlQuery query;
//    query.prepare("SELECT seq FROM sqlite_sequence");
//    query.exec();
//    query.next();
//    qDebug() << query.value(0).toInt();

//    qDebug() << query.lastError() << "\n";


    EchoClient w;
    w.show();

    return a.exec();
}
