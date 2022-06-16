#ifndef IRC42_SERVER_H
#define IRC42_SERVER_H

#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <poll.h>
#include <vector>
#include "Types.hpp"

#define LISTENER_BACKLOG 20
#define NAME_MAX_SZ 10
#define MAX_FDS 255

#define SERVER_BUFF_MAX_SIZE 512

using std::string;
using std::vector;

class User;
class Channel;

namespace irc {


class AddressInfo {
    public:
    AddressInfo(void);
    AddressInfo(AddressInfo &rhs);
    ~AddressInfo(void);

    /* ptr to struct addrinfo list */
    struct addrinfo *servinfo;
    /* actual list entry being used */
    struct addrinfo *actual;
    /* fd set to listen */
    int listener;
};

class FdManager {
    public:
    FdManager(void);
    FdManager(FdManager &rhs);
    ~FdManager(void);

    void setUpListener(int listener);
    bool hasDataToRead(int entry);
    int addNewUser(void);
    /* fd from clients manager. This includes
    * the listener, at entry 0.
    */
    struct pollfd fds[MAX_FDS];
    int fds_size;
    /* addresses corresponding to each client */
    struct sockaddr *addr[MAX_FDS];
};

class Server {

    public:
    Server(void);
    Server(string &ip, string &port);
    Server(Server &rhs);
    ~Server();
    
    private:
    /* Setup */
    int setServerInfo(void);
    int setServerInfo(string &hostname, string &port);
    int setListener(void);
    void loadCommandMap(void);
    void loadNickVector(void);

    /* parses message into commands, calls 
     * commands from user until finished. */
    int MessageFromUser(int fd_idx);
    
    int mainLoop(void);

    typedef enum BUFFER_STATE {
        CLEAN = 0,
        PENDING_DATA = 1,
    } BUFFER_STATE;

    typedef enum COMMAND_RESULT {
        DONE = 0, // nada que hacer
        SEND_ERR_REPLY, // not sure
        ERR_NO_REPLY // not sure either
    } COMMAND_RESULT;
    
    char srv_buff[513];
    int srv_buff_size;
    
    ChannelMap channel_map; /* Find channels by name */
    UserMap user_map;  /* Find users by nickname */
    vector<string> nick_vector; /* Find nickname by fd_idx */

    /* Socket related stuff */
    AddressInfo _info;
    FdManager    _fd_manager;

    /* Map with all comand responses */
    CommandMap cmd_map;

    /* command implementations */
    int NICK(Command &cmd, int fd);
    int USER(Command &cmd, int fd);
};

/**
 * Weechat handshake init Message : [NICK carce
 *   USER carce 0 * :carce
 *  ]
 * 
 * Metodos authenticated
 */

}

#endif /* IRC42_SERVER_H */
