#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QtCore>

struct PROTOCOL{
    static const QChar ADD_MESSAGE;
    static const QChar DELETE_MESSAGE;
    static const QChar MODIFY_MESSAGE;
    static const QChar SET_NEW_MODERATOR;
    static const QChar DELETE_MODERATOR;
    static const QChar LOGIN_OK;
    static const QChar LOGIN_DENIED;
    static const QChar SEND_INIT_MESSAGES;
    static const QChar USER_MODERATOR_LIST;
    static const QChar INIT_MODERATOR;
    static const QChar GOT_MESSAGE_LIST;
    static const QChar USER_GET_PASSWORD_HALF;
};


#endif // PROTOCOL_H
