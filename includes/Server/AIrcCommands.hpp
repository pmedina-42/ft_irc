#ifndef IRC42_AIRCCOMMANDS_H
# define IRC42_AIRCCOMMANDS_H

#include "FdManager.hpp"
#include "IrcDataBase.hpp"

// A stands for Abstract

/* 
 * Esta clase sirve para abstraer las implementaciones de comandos de la 
 * interfaz del servidor.
 * Un Server, entonces, solo tendrá que heredar de AIrcCommands y llamar
 * a loadCommands() con los comandos que se quieran habilitar. 
 *
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

    /* Command implementations */
    void NICK(Command &cmd, int fd);
    void USER(Command &cmd, int fd);
    void PING(Command &cmd, int fd);
    void PONG(Command &cmd, int fd);
    void MODE(Command &cmd, int fd);
    void PASS(Command &cmd, int fd);
    void QUIT(Command &cmd, int fd);
    void JOIN(Command &cmd, int fd);
    void KICK(Command &cmd, int fd);
    void PART(Command &cmd, int fd);
    void TOPIC(Command &cmd, int fd);
    void INVITE(Command &cmd, int fd);
    void NAMES(Command &cmd, int fd);
    void LIST(Command &cmd, int fd);
    void PRIVMSG(Command &cmd, int fd);

    /* Common replies  ? todas privadas ?*/
    void sendWelcome(std::string& name, std::string &prefix, int fd);
    void sendNeedMoreParams(std::string& cmd_name, int fd);
    void sendNotRegistered(std::string &cmd_name, int fd);
    void sendNoSuchChannel(std::string &cmd_name, int fd);
    void sendNotOnChannel(std::string &cmd_name, int fd);
    void sendBadChannelMask(std::string &cmd_name, int fd);
    void sendNoChannelModes(std::string &cmd_name, int fd);
    void sendChannelOperatorNeeded(std::string &cmd_name, int fd);
    void sendAlreadyRegistered(std::string &nick, int fd);
    void sendJoinReply(int fd, User &user, Channel &channel, bool send_all);
    void sendNamesReply(int fd, User &user, Channel &channel);
    void sendListReply(int fd, User &user, std::string ch_name);
    void sendPartMessage(std::string &message, int fd, User &user, Channel &channel);

private:

    void sendMessageToChannel(Channel &channel, std::string &message, std::string &nick);
    std::string constructNamesReply(std::string nick, Channel &channel);
    std::string constructListReply(std::string nick, Channel &channel);

};

} // namespace


#endif /* IRC42_AIRCCOMMANDS_H */
