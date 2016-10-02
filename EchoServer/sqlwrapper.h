#ifndef SQLWRAPPER_H
#define SQLWRAPPER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QWidget>
class QSqlDatabase;
class QSqlQuery;


class SqlWrapper: public QObject
{
    Q_OBJECT

    //"/home/kolegor/Kolepov/Kolepov/EchoClient/server_database.sqlite"

private:
    QSqlDatabase db_connection_;
    static QString path_to_database;
    static QString base_filename;

public:
    SqlWrapper(QObject *parent = 0, const QString& path = SqlWrapper::path_to_database + SqlWrapper::base_filename);
    ~SqlWrapper();

    static void create_database();
};

#endif // SQLWRAPPER_H
