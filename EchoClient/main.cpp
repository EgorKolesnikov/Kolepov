#include "echoclient.h"
#include "connectdialog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    EchoClient w;
    w.show();

    return a.exec();
}
