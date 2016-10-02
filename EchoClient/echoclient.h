#ifndef ECHOCLIENT_H
#define ECHOCLIENT_H

#include <QMainWindow>

class EchoClient : public QMainWindow
{
    Q_OBJECT

public:
    EchoClient(QWidget *parent = 0);
    ~EchoClient();
};

#endif // ECHOCLIENT_H
