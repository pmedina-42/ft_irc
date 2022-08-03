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

class Channel;
class User;
class Command;
class AIrcCommands;

typedef std::map<std::string, irc::Channel> ChannelMap;
typedef std::map<std::string, irc::User> NickUserMap;
typedef std::list<irc::User> UserList;
typedef std::map<std::string, char> ChannelMaskMap;
typedef std::map<std::string, std::string> ChannelModeMap;

typedef std::map<int, irc::User> FdUserMap;
typedef std::map<std::string, int> NickFdMap;

typedef void (irc::AIrcCommands::*CommandFnx)(Command &cmd, int fd);
typedef std::map<std::string, CommandFnx> CommandMap;

}

#endif
