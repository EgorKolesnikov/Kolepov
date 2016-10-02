#ifndef PROTOCOL_H
#define PROTOCOL_H

struct PROTOCOL{
    static const char ADD_MESSAGE = 0;
    static const char DELETE_MESSAGE = 1;
    static const char MODIFY_MESSAGE = 2;
    static const char SET_NEW_MODERATOR = 3;
    static const char DELETE_MODERATOR = 4;
};

#endif // PROTOCOL_H
