#ifndef IRC42_TYPES_H
#define IRC42_TYPES_H

/* Archivo para meter enums, typedefs y mierdas que puedan ser necesarias */

#include <map>
#include <list>
#include <string>

#define CRLF "\r\n"

namespace irc {

class Channel;
class User;
class ChannelUser; // cambiar al nombre que haga falta cuando sea.
class Command;
class Server;

typedef std::map<std::string, irc::Channel> ChannelMap;
typedef std::map<std::string, irc::ChannelUser> ChannelUserMap;
typedef std::list<irc::Command> CommandList;

typedef std::map<int, irc::User> FdUserMap;
typedef std::map<std::string, int> NickFdMap;

typedef int (irc::Server::*CommandFnx)(Command &cmd, int fd); //este es pa enmarcarlo xdd
typedef std::map<std::string, CommandFnx> CommandMap;

}

#endif