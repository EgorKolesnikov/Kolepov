#include "sqlwrapper.h"
#include <QDebug>
#include <QtWidgets>

QString SqlWrapper::path_to_database = "/home/kolegor/Kolepov/Kolepov/EchoClient/";
QString SqlWrapper::base_filename = "server_database.sqlite";

SqlWrapper::SqlWrapper(QObject *parent, const QString& path)
    : QObject(parent)
{
    db_connection_ = QSqlDatabase::addDatabase("QSQLITE");
    db_connection_.setDatabaseName(path);
    db_connection_.open();

    if (!db_connection_.open()) {
        QMessageBox::critical(0, qApp->tr("Cannot open database"),
            qApp->tr("Unable to establish the database connection.\n"
                     "Click Cancel to exit."), QMessageBox::Cancel);
        return;
    }
}

SqlWrapper::~SqlWrapper(){
    if(db_connection_.open()){
        db_connection_.close();
    }
}

void SqlWrapper::create_database(){
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("/home/kolegor/Kolepov/Kolepov/EchoClient/server_database.sqlite");
    if (!db.open()) {
        QMessageBox::critical(0, qApp->tr("Cannot open database"),
            qApp->tr("Unable to establish the database connection.\n"
                     "Click Cancel to exit."), QMessageBox::Cancel);
        return;
    }

    QSqlQuery query;
    query.exec("create table users (id int, name NameAssociated varchar(30))");
    db.close();
}
