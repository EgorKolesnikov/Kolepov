#include "sqlwrapper.h"
#include <QDebug>
#include <QtWidgets>


QString SqlWrapper::path_to_database = "C:\\Users\\1\\Desktop\\projects\\Kolepov\\EchoServer";
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


/*
 *  Test methods.
 */

void SqlWrapper::create_database(){
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(SqlWrapper::path_to_database + SqlWrapper::base_filename);
    if (!db.open()) {
        QMessageBox::critical(0, qApp->tr("Cannot open database"),
            qApp->tr("Unable to establish the database connection.\n"
                     "Click Cancel to exit."), QMessageBox::Cancel);
        return;
    }

    QString query_text = "";
    QSqlQuery query;

    query_text = "CREATE TABLE users ("
                 "user_id integer PRIMARY KEY NOT NULL, "
                 "name VARCHAR(255), "
                 "role VARCHAR(1));";
    query.exec(query_text);

    query.exec("INSERT INTO users VALUES(1, 'Denis', 'a');");
    query.exec("INSERT INTO users VALUES(2, 'Egor', 'u');");
    query.exec("INSERT INTO users VALUES(3, 'Murray', 'u');");
    query.exec("INSERT INTO users VALUES(4, 'Henry', 'u');");
    query.exec("INSERT INTO users VALUES(5, 'Zu', 'u');");

    query_text = "CREATE TABLE messages("
                 "message_id integer PRIMARY KEY NOT NULL, "
                 "message_user_id integer NOT NULL, "
                 "message_text VARCHAR(1024), "
                 "FOREIGN KEY(message_user_id) REFERENCES users(user_id));";
    query.exec(query_text);

    qDebug() << query.lastError().text() << "\n";
    db.close();
}


void SqlWrapper::show_table(const QString& table_name){
    QString query_text = "";
    QSqlQuery query;

    query_text = "SELECT Count(*) FROM " + table_name + ";";
    query.exec(query_text);
    query.first();
    qDebug() << "Rows count: " << query.value(0).toString();

    query_text = "SELECT * FROM " + table_name + ";";
    query.exec(query_text);
    QSqlRecord record_info = query.record();

    if(query.isSelect()){
        while(query.next()){
            QString row = "";
            for(int column = 0; column < record_info.count(); ++column){
                row += query.value(column).toString() + " ";
            }
            qDebug() << row;
        }
    }
}
