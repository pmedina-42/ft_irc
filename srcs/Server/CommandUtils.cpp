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
}