#ifndef SQLWRAPPER_H
#define SQLWRAPPER_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QMutex>
#include <QMutexLocker>

class QSqlDatabase;
class QSqlQuery;
class QSqlError;
class QSqlRecord;
class QMutex;
class QMutexLocker;

class SqlWrapper: public QObject
{
    Q_OBJECT

private:
    QSqlDatabase db_connection_;
    QMutex mutex_;
    static QString path_to_database;
    static QString base_filename;

public:
    SqlWrapper(
            QObject *parent = 0,
            const QString& path = SqlWrapper::path_to_database + SqlWrapper::base_filename
    );
    ~SqlWrapper();

    // Test methods
    static void create_database();
    void show_table(const QString& table_name);

    // Possible queries
    QSqlQuery get_user(const QString& user_name);
    QSqlQuery get_message(int message_id);
    QSqlQuery get_all_messages();

    bool add_message(int user_id, const QString& message_text);
    bool delete_message(int message_id);
    bool modify_message(int message_id, const QString& new_message_text);
    bool change_user_priveledge(int user_id, char new_role);
};

#endif // SQLWRAPPER_H
