#include "Server/Server.hpp"
#include "Command.hpp"
#include "NumericReplies.hpp"

using std::string;

namespace irc {

void Server::sendNeedMoreParams(string& cmd_name, int fd) {
    string reply(ERR_NEEDMOREPARAMS+cmd_name+STR_NEEDMOREPARAMS);
    DataToUser(fd, reply, NUMERIC_REPLY);
}

void Server::sendNotRegistered(string &cmd_name, int fd) {
    string reply(ERR_NOTREGISTERED+cmd_name+STR_NOTREGISTERED);
    DataToUser(fd, reply, NUMERIC_REPLY);
}

void Server::sendNoSuchChannel(string &cmd_name, int fd) {
    string reply = (ERR_NOSUCHCHANNEL+cmd_name+STR_NOSUCHCHANNEL);
    DataToUser(fd, reply, NUMERIC_REPLY);
}

void Server::sendNotOnChannel(string &cmd_name, int fd) {
    string reply = (ERR_NOTONCHANNEL+cmd_name+STR_NOTONCHANNEL);
    DataToUser(fd, reply, NUMERIC_REPLY);
}

void Server::sendBadChannelMask(string &cmd_name, int fd) {
    string reply = (ERR_BADCHANMASK+cmd_name+STR_BADCHANMASK);
    DataToUser(fd, reply, NUMERIC_REPLY);
}

void Server::sendNoChannelModes(string &cmd_name, int fd) {
    string reply = (ERR_NOCHANMODES+cmd_name+STR_NOCHANMODES);
    DataToUser(fd, reply, NUMERIC_REPLY);
}

void Server::sendChannelOperatorNeeded(string &cmd_name, int fd) {
    string reply = (ERR_CHANOPRIVSNEEDED+cmd_name+STR_CHANOPRIVSNEEDED);
    DataToUser(fd, reply, NUMERIC_REPLY);
}

void Server::sendWelcome(string& nick, string &prefix, int fd) {
    string welcome_msg(RPL_WELCOME+nick+RPL_WELCOME_STR_1+prefix);
    DataToUser(fd, welcome_msg, NUMERIC_REPLY);
}

void Server::sendAlreadyRegistered(string &nick, int fd) {
    string reply = (ERR_ALREADYREGISTERED+nick+STR_ALREADYREGISTERED);
    DataToUser(fd, reply, NUMERIC_REPLY);
}


} /* namespace irc */
