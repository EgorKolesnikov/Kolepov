#ifndef SQLWRAPPER_H
#define SQLWRAPPER_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>

class QSqlDatabase;
class QSqlQuery;
class QSqlError;
class QSqlRecord;

class SqlWrapper: public QObject
{
    Q_OBJECT

private:
    QSqlDatabase db_connection_;
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
    void show_messages();
};

#endif // SQLWRAPPER_H
