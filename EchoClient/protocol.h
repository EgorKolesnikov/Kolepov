#ifndef PROTOCOL_H
#define PROTOCOL_H


struct PROTOCOL{
    static const QChar ADD_MESSAGE;
    static const QChar DELETE_MESSAGE;
    static const QChar MODIFY_MESSAGE;
    static const QChar SET_NEW_MODERATOR;
    static const QChar DELETE_MODERATOR;
    static const QChar LOGIN_OK;
    static const QChar LOGIN_DENIED;
};

const QChar PROTOCOL::ADD_MESSAGE = 0;
const QChar PROTOCOL::DELETE_MESSAGE = 1;
const QChar PROTOCOL::MODIFY_MESSAGE = 2;
const QChar PROTOCOL::SET_NEW_MODERATOR = 3;
const QChar PROTOCOL::DELETE_MODERATOR = 4;
const QChar PROTOCOL::LOGIN_OK = 5;
const QChar PROTOCOL::LOGIN_DENIED = 6;

#endif // PROTOCOL_H
