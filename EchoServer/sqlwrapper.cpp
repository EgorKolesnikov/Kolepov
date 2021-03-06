#include "sqlwrapper.h"
#include <QDebug>
#include <QtWidgets>


class Sleeper: public QThread
{
    public:
        static void msleep(int ms)
        {
            QThread::msleep(ms);
        }
};


QString SqlWrapper::path_to_database = "/home/kolegor/Code/Kolepov/EchoServer/";
//QString SqlWrapper::path_to_database = "C:\\Users\\1\\Desktop\\projects\\Kolepov\\EchoServer\\";
QString SqlWrapper::base_filename = "server_database.sqlite";


SqlWrapper::SqlWrapper(QObject *parent, QString database_password, const QString& path)
    : QObject(parent)
    , mutex_()
{
    db_connection_ = QSqlDatabase::addDatabase("QSQLITE"); // QSqlDatabase::addDatabase("SQLITECIPHER");
    db_connection_.setDatabaseName(path);
    db_connection_.setPassword(kdf(database_password));

    if (!db_connection_.open()) {
        QMessageBox::critical(0, qApp->tr("Cannot open database."),
            qApp->tr("Unable to establish the database connection.\n"
                     "%1").arg(db_connection_.lastError().text()),
                              QMessageBox::Cancel);
        return;
    }
}


SqlWrapper::~SqlWrapper(){
    if(db_connection_.isOpen()){
        db_connection_.close();
    }
}

QString SqlWrapper::kdf(QString &string_password){
    QByteArray array(string_password.toStdString().c_str());
    byte* password = reinterpret_cast<byte*>(array.data());
    size_t plen = strlen((const char*)password);

    byte salt[] = "salt";
    size_t slen = strlen((const char*)salt);
    size_t dlen = 128;
    byte derived[dlen];

    // CryptoPP::HKDF<CryptoPP::SHA1> pbkdf2;
    // pbkdf2.DeriveKey(derived, dlen, password, plen, salt, slen);

    CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA1> pbkdf2;
    pbkdf2.DeriveKey(derived, sizeof(derived), 0, password, plen, salt, slen, 1);
    return QString((const char*)derived);
}

const int wait = 100;


/*
 *  Possible queries.
 */

QSqlQuery SqlWrapper::get_user(const QString &user_name){
    QMutexLocker locker(&mutex_);
    // Sleeper::msleep(wait);

    QSqlQuery query;
    query.prepare("SELECT * FROM users WHERE name=?;");
    query.bindValue(0, user_name);
    query.exec();

    return query;
}

QSqlQuery SqlWrapper::get_message(int message_id){
    QMutexLocker locker(&mutex_);
   // Sleeper::msleep(wait);

    QSqlQuery query;
    query.prepare("SELECT * FROM messages WHERE message_id=:m_id;");
    query.bindValue(":m_id", message_id);
    query.exec();

    return query;
}

int SqlWrapper::get_message_id(int user_id, const QString& message){
    QMutexLocker locker(&mutex_);
   // Sleeper::msleep(wait);

    QSqlQuery query;
    query.prepare("SELECT MAX(message_id) AS m_id "
                  "FROM messages "
                  "WHERE text=:m_txt AND user_id=:u_id;");
    query.bindValue(":m_txt", message);
    query.bindValue(":u_id", user_id);
    query.exec();
    query.next();
    return query.value("m_id").toInt();
}

QSqlQuery SqlWrapper::get_all_messages()
{
    QMutexLocker locker(&mutex_);
    //Sleeper::msleep(wait);

    QSqlQuery query;
    query.exec("SELECT name, message_id, text "
               "FROM messages NATURAL JOIN users;");

    return query;
}

QSqlQuery SqlWrapper::get_all_users()
{
    QMutexLocker locker(&mutex_);
    return QSqlQuery("SELECT name, role FROM users;");
}

bool SqlWrapper::add_message(int user_id, const QString &message_text){
    QMutexLocker locker(&mutex_);
    //Sleeper::msleep(wait);

    QSqlQuery query;
    query.prepare("INSERT INTO messages (user_id, text) "
                  "VALUES(:u_id, :m_txt);");
    query.bindValue(":u_id", user_id);
    query.bindValue(":m_txt", message_text);

    return query.exec();
}

bool SqlWrapper::delete_message(int message_id){
    QMutexLocker locker(&mutex_);
    //Sleeper::msleep(wait);

    QSqlQuery query;
    query.prepare("DELETE FROM messages WHERE message_id=:m_id;");
    query.bindValue(":m_id", message_id);

    return query.exec();
}

bool SqlWrapper::modify_message(int message_id, const QString &new_message_text){
    QMutexLocker locker(&mutex_);
    //Sleeper::msleep(wait);

    QSqlQuery query;
    query.prepare("UPDATE messages SET text=:new_m_text WHERE message_id=:m_id;");
    query.bindValue(":new_m_text", new_message_text);
    query.bindValue(":m_id", message_id);

    return query.exec();
}

bool SqlWrapper::change_user_role(const QString &user_name, QString new_role){
    QMutexLocker locker(&mutex_);
    //Sleeper::msleep(wait);

    QSqlQuery query;
    query.prepare("UPDATE users SET role=:n_role WHERE name=:u_name;");
    query.bindValue(":n_role", new_role);
    query.bindValue(":u_name", user_name);

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

    query.exec("DROP TABLE users;");
    query.exec("DROP TABLE messages;");

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
                 "user_id integer NOT NULL, "
                 "text VARCHAR(1024), "
                 "FOREIGN KEY(user_id) REFERENCES users(user_id));";
    query.exec(query_text);

    query.exec("INSERT INTO messages (user_id, text) VALUES (2, 'Test message');");

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

QString SqlWrapper::get_user_password_half(const QString &username){
    QMutexLocker locker(&mutex_);
    // Sleeper::msleep(wait);

    QSqlQuery query;
    query.prepare("SELECT servers_password_part FROM users WHERE name=?;");
    query.bindValue(0, username);
    query.exec();
    query.next();

    QString result = query.value(0).toString();
    qDebug() << result << "\n";

    return result;
}
