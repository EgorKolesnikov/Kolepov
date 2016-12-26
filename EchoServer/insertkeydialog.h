#ifndef INSERTKEYDIALOG_H
#define INSERTKEYDIALOG_H

#include <QDialog>
#include <QLineEdit>

class InsertKeyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InsertKeyDialog(QWidget *parent = Q_NULLPTR);
    QString getChoosenPath();


private slots:
    void browseServerKeyFile();
    void runServer();

private:
    QLineEdit *keyPathLineEdit;
    QPushButton *connectButton;
};

#endif // INSERTKEYDIALOG_H
