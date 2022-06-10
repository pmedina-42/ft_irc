#ifndef IRC42_SERVER_H
#define IRC42_SERVER_H

#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <poll.h>

#define LISTENER_BACKLOG 20
#define NAME_MAX_SZ 10
#define MAX_FDS 255

using std::string;

class User;

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
    int hasDataToRead(int entry);
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
    /* initializators */
    int setServerInfo(void);
    int setServerInfo(string &hostname, string &port);
    int setListener(void);
    
    int mainLoop(void);
    
    void printError(string error);
    AddressInfo _info;
    FdManager    _fd_manager;
};

/**
 * fd : 2Message : [NICK carce
 *   USER carce 0 * :carce
 *  ]
 * 
 */

}

#endif /* IRC42_SERVER_H */
