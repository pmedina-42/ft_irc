#include "Server/AIrcCommands.hpp"
#include "NumericReplies.hpp"

using std::string;

namespace irc {

void AIrcCommands::sendNeedMoreParams(string& cmd_name, int fd) {
    string reply(ERR_NEEDMOREPARAMS+cmd_name+STR_NEEDMOREPARAMS);
    DataToUser(fd, reply, NUMERIC_REPLY);
}

void AIrcCommands::sendNotRegistered(string &cmd_name, int fd) {
    string reply(ERR_NOTREGISTERED+cmd_name+STR_NOTREGISTERED);
    DataToUser(fd, reply, NUMERIC_REPLY);
}

void AIrcCommands::sendNoSuchChannel(string &cmd_name, int fd) {
    string reply = (ERR_NOSUCHCHANNEL+cmd_name+STR_NOSUCHCHANNEL);
    DataToUser(fd, reply, NUMERIC_REPLY);
}

void AIrcCommands::sendNotOnChannel(string &cmd_name, int fd) {
    string reply = (ERR_NOTONCHANNEL+cmd_name+STR_NOTONCHANNEL);
    DataToUser(fd, reply, NUMERIC_REPLY);
}

void AIrcCommands::sendBadChannelMask(string &cmd_name, int fd) {
    string reply = (ERR_BADCHANMASK+cmd_name+STR_BADCHANMASK);
    DataToUser(fd, reply, NUMERIC_REPLY);
}

void AIrcCommands::sendNoChannelModes(string &cmd_name, int fd) {
    string reply = (ERR_NOCHANMODES+cmd_name+STR_NOCHANMODES);
    DataToUser(fd, reply, NUMERIC_REPLY);
}

void AIrcCommands::sendChannelOperatorNeeded(string &cmd_name, int fd) {
    string reply = (ERR_CHANOPRIVSNEEDED+cmd_name+STR_CHANOPRIVSNEEDED);
    DataToUser(fd, reply, NUMERIC_REPLY);
}

void AIrcCommands::sendWelcome(string& nick, string &prefix, int fd) {
    string welcome_msg(RPL_WELCOME+nick+RPL_WELCOME_STR_1+prefix);
    DataToUser(fd, welcome_msg, NUMERIC_REPLY);
    // TODO fix these common replies
    string welcome_msg2(RPL_YOURHOST""STR_YOURHOST);
    DataToUser(fd, welcome_msg2, NUMERIC_REPLY);
    string welcome_msg3(RPL_CREATED""STR_CREATED);
    DataToUser(fd, welcome_msg3, NUMERIC_REPLY);
    string welcome_msg4(RPL_MYINFO""STR_MYINFO);
    DataToUser(fd, welcome_msg4, NUMERIC_REPLY);
}

void AIrcCommands::sendAlreadyRegistered(string &nick, int fd) {
    string reply = (ERR_ALREADYREGISTERED+nick+STR_ALREADYREGISTERED);
    DataToUser(fd, reply, NUMERIC_REPLY);
}


} /* namespace irc */
