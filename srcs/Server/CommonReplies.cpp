#include "Server/Server.hpp"
#include "Command.hpp"
#include "NumericReplies.hpp"

namespace irc {

void Server::sendNeedMoreParams(string& cmd_name, int fd_idx) {
    string reply(ERR_NEEDMOREPARAMS+cmd_name+STR_NEEDMOREPARAMS);
    DataToUser(fd_idx, reply, NUMERIC_REPLY);
}

void Server::sendNotRegistered(string &cmd_name, int fd_idx) {
    string reply(ERR_NOTREGISTERED+cmd_name+STR_NOTREGISTERED);
    DataToUser(fd_idx, reply, NUMERIC_REPLY);
}

void Server::sendNoSuchChannel(string &cmd_name, int fd_idx) {
    string reply = (ERR_NOSUCHCHANNEL+cmd_name+STR_NOSUCHCHANNEL);
    DataToUser(fd_idx, reply, NUMERIC_REPLY);
}

void Server::sendNotOnChannel(string &cmd_name, int fd_idx) {
    string reply = (ERR_NOTONCHANNEL+cmd_name+STR_NOTONCHANNEL);
    DataToUser(fd_idx, reply, NUMERIC_REPLY);
}

void Server::sendBadChannelMask(string &cmd_name, int fd_idx) {
    string reply = (ERR_BADCHANMASK+cmd_name+STR_BADCHANMASK);
    DataToUser(fd_idx, reply, NUMERIC_REPLY);
}

void Server::sendNoChannelModes(string &cmd_name, int fd_idx) {
    string reply = (ERR_NOCHANMODES+cmd_name+STR_NOCHANMODES);
    DataToUser(fd_idx, reply, NUMERIC_REPLY);
}

void Server::sendChannelOperatorNeeded(string &cmd_name, int fd_idx) {
    string reply = (ERR_CHANOPRIVSNEEDED+cmd_name+STR_CHANOPRIVSNEEDED);
    DataToUser(fd_idx, reply, NUMERIC_REPLY);
}

void Server::sendWelcome(string& name, string &prefix, int fd_idx) {
    string welcome_msg(RPL_WELCOME+name+RPL_WELCOME_STR_1+prefix);
    DataToUser(fd_idx, welcome_msg, NUMERIC_REPLY);
}

void Server::sendAlreadyRegistered(int fd_idx) {
    string reply = (ERR_ALREADYREGISTERED "" STR_ALREADYREGISTERED);
    DataToUser(fd_idx, reply, NUMERIC_REPLY);
}


} /* namespace irc */
