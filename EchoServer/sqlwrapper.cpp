#include "sqlwrapper.h"
#include <QDebug>
#include <QtWidgets>


QString SqlWrapper::path_to_database = "C:\\Users\\1\\Desktop\\projects\\Kolepov\\EchoServer";
QString SqlWrapper::base_filename = "server_database.sqlite";


SqlWrapper::SqlWrapper(QObject *parent, const QString& path)
    : QObject(parent)
    , mutex_()
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
 *  Possible queries.
 */

QSqlQuery SqlWrapper::get_user(const QString &user_name){
    QMutexLocker locker(&mutex_);

    QSqlQuery query;
    query.prepare("SELECT * FROM users WHERE name=:name;");
    query.bindValue(":name", user_name);
    query.exec();

    return query;
}

QSqlQuery SqlWrapper::get_message(int message_id){
    QMutexLocker locker(&mutex_);

    QSqlQuery query;
    query.prepare("SELECT * FROM messages WHERE message_id=:m_id;");
    query.bindValue(":m_id", message_id);
    query.exec();

    return query;
}

bool SqlWrapper::add_message(int user_id, const QString &message_text){
    QMutexLocker locker(&mutex_);

    QSqlQuery query;
    query.prepare("INSERT INTO messages (message_user_id, message_text) "
                  "VALUES(message_user_id=:user_id, message_text=:m_text);");
    query.bindValue(":m_id", message_id);
    query.bindValue(":m_text", message_text);

    return query.exec();
}

bool SqlWrapper::delete_message(int message_id){
    QMutexLocker locker(&mutex_);

    QSqlQuery query;
    query.prepare("DELETE FROM messages WHERE message_id=:m_id;");
    query.bindValue(":m_id", message_id);

    return query.exec();
}

bool SqlWrapper::modify_message(int message_id, const QString &new_message_text){
    QMutexLocker locker(&mutex_);

    QSqlQuery query;
    query.prepare("UPDATE messages SET message_text=:new_m_text WHERE message_id=:m_id;");
    query.bindValue(":new_n_text", new_message_text);
    query.bindValue(":m_id", message_id);

    return query.exec();
}

bool SqlWrapper::change_user_priveledge(int user_id, char new_role){
    QMutexLocker locker(&mutex_);

    QSqlQuery query;
    query.prepare("UPDATE users SET role=:role WHERE use_id=:u_id;");
    query.bindValue(":role", new_role);
    query.bindValue(":u_id", user_id);

    return query.exec();
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
                 "user_id integer PRIMARY KEY AUTOINCREMENT NOT NULL UNIQUE, "
                 "name VARCHAR(255), "
                 "role VARCHAR(1));";
    query.exec(query_text);

    query.exec("INSERT INTO users (name, role) VALUES('Denis', 'a');");
    query.exec("INSERT INTO users (name, role) VALUES('Egor', 'u');");
    query.exec("INSERT INTO users (name, role) VALUES('Murray', 'u');");
    query.exec("INSERT INTO users (name, role) VALUES('Henry', 'u');");
    query.exec("INSERT INTO users (name, role) VALUES('Zu', 'u');");

    query_text = "CREATE TABLE messages("
                 "message_id integer PRIMARY KEY AUTOINCREMENT NOT NULL UNIQUE, "
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
