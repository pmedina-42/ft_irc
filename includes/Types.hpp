#ifndef IRC42_TYPES_H
# define IRC42_TYPES_H

/* Archivo para meter enums, typedefs y mierdas que puedan ser necesarias */

#include <map>
#include <list>
#include <string>

#define CRLF "\r\n"
#define CR "\r"
#define LF "\n"

/*
 * On why enums are chosen over macros for integers :
 * https://stackoverflow.com/questions/3134757/
 */

namespace irc {

typedef enum {
    LISTENER_BACKLOG = 20,
    NAME_MAX_SIZE = 10,
    MAX_FDS = 255,
    POLL_TIMEOUT_MS = 1000,
    BUFF_MAX_SIZE = 512,
    PING_TIMEOUT_S = 120 
} SERVER_CONFIG;

typedef enum {
    NO_NUMERIC_REPLY = 0,
    NUMERIC_REPLY
} FLAG_NUMERIC;

typedef enum {
    OPER = 7,
    CH_MOD = 6,
    SV_AWAY = 6
} IRC_MODES;

class Channel;
class User;
class Command;
class AIrcCommands;

}

#endif
