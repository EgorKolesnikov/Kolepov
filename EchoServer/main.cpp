#include <QApplication>
#include <QtCore>
#include <stdlib.h>
#include "echoserver.h"


int main(int argc, char *argv[])
{
    // QString qwe = "password";
    // QByteArray array(qwe.toStdString().c_str());
    // byte * password = reinterpret_cast<byte*>(array.data());
    // size_t plen = strlen((const char*)password);
    // byte salt[] = "salt";
    // size_t slen = strlen((const char*)salt);
    // int c = 1;
    // byte derived[30];
    // CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA1> pbkdf2;
    // pbkdf2.DeriveKey(derived, sizeof(derived), 0, password, plen, salt, slen, c);
    // qDebug() << QString((const char*)derived);

    QApplication app(argc, argv);
    QGuiApplication::setApplicationDisplayName(
                EchoServer::tr("Server"));

    EchoServer server;
    server.show();

    return app.exec();

//    SqlWrapper::create_database();
//    return 0;
}
