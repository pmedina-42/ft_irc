#ifndef SERVER_H
#define SERVER_H

#include "User.hpp"
#include <vector>
#include <unistd.h>
#include <netinet/in.h>
#include <poll.h>

#define LISTENER_BACKLOG 20
#define NAME_MAX_SZ 10
#define MAX_FDS 255

class User;

namespace irc {

class serverParams;
class serverFds;

class Server {

    public:
        Server(void);
        Server(std::string &ip, std::string &port);
        Server(Server &rhs);
        ~Server();
    
    private:
        /* initializators */
        int setServerInfo(void);
        int setServerInfo(std::string &ip, std::string &port);
        int setListener(void);

        int mainLoop(void);
        
        void printError(std::string error);
        serverParams& _info;
        serverFds&     _manager;
};

class serverParams {
    public:
        serverParams(void);
        serverParams(serverParams &rhs);
        ~serverParams(void);

        /* ptr to struct addrinfo list */
        struct addrinfo *servinfo;
        /* actual list entry being used */
        struct addrinfo *actual;
        /* fd set to listen */
        int listener;
};

class serverFds {
    public:
        serverFds(void);
        serverFds(serverFds &rhs);
        ~serverFds(void);

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

}

#endif
