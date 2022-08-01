#ifndef IRC42_AIRCCOMMANDS_H
# define IRC42_AIRCCOMMANDS_H

#include "FdManager.hpp"
#include "IrcDataBase.hpp"

// A stands for Abstract

/* This class is to abstract the command implementations from the
 * Server interface. Servers will then only have to inherit from AIrcCommands,
 * and when adding/substracting commands from AIrcCommands, the Server will
 * remain the same. 
 */

namespace irc {

class AIrcCommands : public FdManager,
                     public IrcDataBase
{
    public:
    AIrcCommands(void);
    AIrcCommands(std::string &ip, std::string &port);
    AIrcCommands(const AIrcCommands &other);
    ~AIrcCommands();

    /* Provided by Server */
    virtual void DataFromUser(int fd) = 0;
    virtual void DataToUser(int fd, std::string &data, int type) = 0;
    virtual void loadCommandMap(void) = 0;

    void NICK(Command &cmd, int fd);
    void USER(Command &cmd, int fd);
    void PING(Command &cmd, int fd);
    void PONG(Command &cmd, int fd);
    void MODE(Command &cmd, int fd);
    void PASS(Command &cmd, int fd);
    void AWAY(Command &cmd, int fd);
    void QUIT(Command &cmd, int fd);
    void JOIN(Command &cmd, int fd);
    void KICK(Command &cmd, int fd);
    void PART(Command &cmd, int fd);
    void TOPIC(Command &cmd, int fd);
    void INVITE(Command &cmd, int fd);

    /* Common replies */
    void sendWelcome(std::string& name, std::string &prefix, int fd);
    void sendNeedMoreParams(std::string& cmd_name, int fd);
    void sendNotRegistered(std::string &cmd_name, int fd);
    void sendNoSuchChannel(std::string &cmd_name, int fd);
    void sendNotOnChannel(std::string &cmd_name, int fd);
    void sendBadChannelMask(std::string &cmd_name, int fd);
    void sendNoChannelModes(std::string &cmd_name, int fd);
    void sendChannelOperatorNeeded(std::string &cmd_name, int fd);
    void sendAlreadyRegistered(std::string &nick, int fd);
};

} // namespace


#endif /* IRC42_AIRCCOMMANDS_H */