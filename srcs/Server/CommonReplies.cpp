#include "Server/AIrcCommands.hpp"
#include "NumericReplies.hpp"
#include "Channel.hpp"
#include "User.hpp"
#include "libft.h"
#include "Exceptions.hpp"

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

void AIrcCommands::sendPasswordMismatch(string &nick, int fd) {
    string reply = (ERR_PASSWDMISMATCH+nick+STR_PASSWDMISMATCH);
    DataToUser(fd, reply, NUMERIC_REPLY);
}

void AIrcCommands::sendJoinReply(int fd, User &user, Channel &channel, bool send_all) {
    string join_rpl = ":" + user.prefix + " JOIN :" + channel.name;
    if (send_all) {
        sendMessageToChannel(channel, join_rpl, user.nick);
    }
    if (!channel.topic.empty()) {
        string topic_rpl = ":" + user.prefix + " " + channel.name + " :" + channel.topic;
        DataToUser(fd, topic_rpl, NO_NUMERIC_REPLY);
    }
    DataToUser(fd, join_rpl, NO_NUMERIC_REPLY);
    sendNamesReply(fd, user, channel);
}

void AIrcCommands::sendNamesReply(int fd, User &user, Channel &channel) {
    string namesReply = constructNamesReply(user.real_nick, channel);
    DataToUser(fd, namesReply, NUMERIC_REPLY);
    namesReply = (RPL_ENDOFNAMES + user.real_nick + " "
                  + channel.name
                  + STR_ENDOFNAMES);
    DataToUser(fd, namesReply, NUMERIC_REPLY);
}

void AIrcCommands::sendListReply(int fd, User &user, string ch_name) {
    string start_rpl = (RPL_LISTSTART + user.real_nick + STR_LISTSTART);
    DataToUser(fd, start_rpl, NUMERIC_REPLY);
    if (ch_name.compare("")) {
        LOG(DEBUG) << ch_name;
        LOG(DEBUG) << "2";
        Channel &channel = channel_map.find(ch_name)->second;
        string reply = constructListReply(user.real_nick, channel);
        DataToUser(fd, reply, NUMERIC_REPLY);
    } else if (channel_map.size() > 0) {
        LOG(DEBUG) << "3";
        for (std::map<std::string, Channel>::iterator it = channel_map.begin(); it != channel_map.end(); it++) {
            string reply = constructListReply(user.real_nick, it->second);
            DataToUser(fd, reply, NUMERIC_REPLY);
        }
    }
    string end_reply = (RPL_LISTEND + user.real_nick + STR_LISTEND);
    DataToUser(fd, end_reply, NUMERIC_REPLY);
}

// TODO: part message en weechat no funciona igual que me devuelve irchispano
void AIrcCommands::sendPartMessage(string &message,  int fd, User &user, Channel &channel) {
    string part_message = message.compare("") ? " :\"" + message.substr(1) + "\"" : "";
    string part_rpl = ":" + user.prefix + " PART :" + channel.name + part_message;
    sendMessageToChannel(channel, part_rpl, user.nick);
    return (DataToUser(fd, part_rpl, NO_NUMERIC_REPLY));
}


// PRIVATE METHODS

void AIrcCommands::sendMessageToChannel(Channel &channel, string &message, string &nick) {
    for (std::list<string>::iterator it = channel.users.begin(); it != channel.users.end(); it++) {
        User &receiver = getUserFromNick(*it);
        if (receiver.nick.compare(nick)) {
            DataToUser(receiver.fd, message, NO_NUMERIC_REPLY);
        }
    }
}

string AIrcCommands::constructNamesReply(string nick, Channel &channel) {
    unsigned long i = 0;
    string reply = (RPL_NAMREPLY + nick + " = " + channel.name + " " + ":");
    for (std::list<string>::iterator it = channel.users.begin(); it != channel.users.end(); it++) {
        User user = getUserFromNick(*it);
        if (user.isChannelOperator(channel.name)) {
            reply += "@";
        }
        reply += user.real_nick;
        reply += (++i < channel.users.size()) ? " " : "";
    }
    return reply;
}

string AIrcCommands::constructListReply(string nick, Channel &channel) {
    string mode = channel.keyModeOn() ? "k" : "";
    mode += channel.inviteModeOn() ? "i" : "";
    mode += channel.moderatedModeOn() ? "m" : "";
    mode = mode.length() != 0 ? ("+" + mode) : mode;
    char* channel_size = ft_itoa((int)channel.users.size());
    if (!channel_size) {
        throw irc::exc::MallocError();
    }
    string reply = (RPL_LIST + nick + " " + channel.name + " " + std::string(channel_size) + " [" + mode + (mode == "" ? "+t]" : "t]"));
    reply += channel.topicModeOn() ? (" " + channel.topic) : "";
    free(channel_size);
    return reply;
}

} /* namespace irc */
