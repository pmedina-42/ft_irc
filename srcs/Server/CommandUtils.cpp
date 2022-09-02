#include "Server/AIrcCommands.hpp"
#include "NumericReplies.hpp"
#include "Channel.hpp"
#include "User.hpp"
#include "Command.hpp"

namespace irc {

void AIrcCommands::createNewChannel(const Command &cmd, int size, User &user, int fd) {
    Channel channel(cmd.args[1], user);
    user.ch_name_mask_map.insert(
            std::pair<std::string, unsigned char>(channel.name, 0x80));
    if (size >= 3) {
        channel.key = cmd.args[2];
        channel.addMode(CH_PAS);
    }
    addNewChannel(channel);
    sendJoinReply(fd, user, channel, false);
}

void AIrcCommands::sendWhoisReply(const Command &cmd, int fd, User &user, std::string &nick) {
    User &whois = getUserFromNick(nick);
    std::string info_rpl = (RPL_WHOISUSER+user.real_nick+" "+whois.real_nick
                       +" "+whois.name+" "+whois.ip_address+STR_WHOISUSER+whois.full_name);
    DataToUser(fd, info_rpl, NUMERIC_REPLY);
    if (!whois.ch_name_mask_map.empty()) {
        std::string channel_rpl = constructWhoisChannelRpl(whois, user.real_nick);
        DataToUser(fd, channel_rpl, NUMERIC_REPLY);
    }
    std::string mid_rpl = (RPL_WHOISSERVER + user.real_nick + " " + whois.real_nick
                      + " " + hostname + " :WhatsApp 2");
    DataToUser(fd, mid_rpl, NUMERIC_REPLY);
    std::string end_rpl = (RPL_ENDOFWHOIS + user.real_nick + " " + cmd.args[1] + STR_ENDOFWHOIS);
    DataToUser(fd, end_rpl, NUMERIC_REPLY);
}

void AIrcCommands::joinExistingChannel(int fd, User &user, Channel &channel) {
    channel.addUser(user);
    user.ch_name_mask_map.insert(std::pair<std::string, unsigned char>(channel.name, 0x00));
    if (channel.topicModeOn() && !channel.topic.empty()) {
        std::string reply(RPL_TOPIC+user.nick+" "+channel.name+" :"+channel.topic);
        DataToUser(fd, reply, NUMERIC_REPLY);
    }
    sendJoinReply(fd, user, channel, true);
}

}

