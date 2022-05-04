#ifndef SERVER_H
#define SERVER_H

#include "User.hpp"
#include <vector>
#include <unistd.h>
#include <netinet/in.h>
#include <poll.h>

#define LISTENER_BACKLOG 20
#define NAME_MAX_SZ 10

class User;

namespace irc {

class serverParams;

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
        
        void printError(std::string error);
        serverParams& _info;
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

}

#endif
