#ifndef INSERTKEYDIALOG_H
#define INSERTKEYDIALOG_H

#include <QDialog>
#include <QLineEdit>

class InsertKeyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InsertKeyDialog(QWidget *parent = Q_NULLPTR);
    QString getChoosenServerKeyPath();
    QString getChoosenDatabaseKeyPath();
    void enableConnectButton();


private slots:
    void browseServerKeyFile();
    void runServer();

private:
    QLineEdit *keyPathLineEdit;
    QLineEdit *keyDatabasePathLineEdit;
    QPushButton *connectButton;
};

#endif // INSERTKEYDIALOG_H
