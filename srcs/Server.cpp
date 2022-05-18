#include <sys/utsname.h>
#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>

#include "../includes/Server.hpp"
#include "../includes/User.hpp"

using std::string;

/* ref https://www.youtube.com/watch?v=dEHZb9JsmOU
 * 
 * IRC 'handshake'
 * http://chi.cs.uchicago.edu/chirc/irc_examples.html
 * 
 * install Weechat (Hell) on Ubuntu
 * https://weechat.org/files/doc/stable/weechat_user.en.html#dependencies
 * https://weechat.org/files/doc/stable/weechat_user.en.html
 * 
 * Userguide to Weechat:
 * https://www.linode.com/docs/guides/using-weechat-for-irc/
 */

namespace irc {

int get_addrinfo_from_params(const char* hostname, const char *port,
                             struct addrinfo *hints,
                             struct addrinfo **servinfo)
{
    int ret = -1;

    if ((ret = getaddrinfo(hostname, port, hints, servinfo)) != 0) {
        string error("getaddrinfo error :");
        std::cerr << error.append(gai_strerror(ret)) << std::endl;
        return -1;
    }
    /* Filter out IPv6 cases (makes me dizzy) */
    if ((*servinfo)->ai_family == AF_INET6) {
        std::cerr << "unsupported IP address length" << std::endl;
        return -1;
    }
    return 0;
}

/* When called, two modes exist:
 * MANUAL : prints the first IP address returned by gethostbyname,
 *          to allow a further call to the server initializer 
 *          using the IP provided.
 * AUTOMATIC : the server mounts automatically on an address available.
 */
Server::Server(void)
    :
        _info(),
        _manager()
{
    if (setServerInfo() == -1
        || setListener() == -1)
    {
        // maybe throw ?
        exit(1);
    }
    mainLoop();
}

Server::Server(string &hostname, string &port) {
    if (setServerInfo(hostname, port) != 0
        || setListener() == -1)
    {
        std::cout << "Error setting Server Info" << std::endl;
    }
    mainLoop();
}

Server::~Server(void) {
}


/*
 * sets all entries in internal struct addrinfo. This includes
 * hostname, the list of addresses to be used, the IRC port,
 * the transport protocol and the IP protocol.
 * Since this function is
 * essential to the server functionality, shuts down the 
 * program in case of failure.
 * 
 * returns 0 on success, 1 otherwise. 
 */
int Server::setServerInfo(void) {
    struct addrinfo hints;
    struct addrinfo *servinfo = NULL;
    char hostname[96];
    size_t hostname_len = 96;
    const char* port = "6667";

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // Ipv4
    hints.ai_socktype = SOCK_STREAM; // TCP

    if (gethostname(&hostname[0], hostname_len) != 0) {
        printError("gethostname error");
        return -1;
    }
    std::cout << hostname << std::endl;
    if (get_addrinfo_from_params(hostname, port, &hints, &servinfo) == -1) {
        if (servinfo != NULL) {
            freeaddrinfo(servinfo);
            return -1;
        }
    }
    if (servinfo == NULL)
        return -1;
    /* This struct addrinfo may not be the one that binds, but
     * thats one more step from here. */
    _info.servinfo = servinfo;
    return 0;
}

/*
 * Here ip works as the hostname. It is already null terminated
 * when calling string c_str method.
 */
int Server::setServerInfo(string &hostname, string &port) {
    struct addrinfo hints;
    struct addrinfo *servinfo = NULL;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // Ipv4
    hints.ai_socktype = SOCK_STREAM; // TCP
    
    /* in_addr and sockaddr_in are both just uint32_t */
    //if (inet_aton(hostname.c_str(), (in_addr *)hints.ai_addr) == 0)
    //	return -1;

    if (get_addrinfo_from_params(hostname.c_str(), port.c_str(), &hints,
                                 &servinfo) == -1)
    {
        if (servinfo != NULL) {
            freeaddrinfo(servinfo);
            return -1;
        }
    }
    if (servinfo == NULL)
        return -1;
    /* This struct addrinfo may not be the one that binds, but
     * thats one more step from here. */
    _info.servinfo = servinfo;
    return 0;
}

/*
 * tries to bind a socket to one of the provided addresses in
 * the struct addrinfo list provided by servinfo and then, it 
 * starts listen()ing to it.
 * return 0 on success, -1 otherwise.
 */
int Server::setListener(void) {

    int socketfd = -1;
    /* loop through addresses until bind works */
    for (struct addrinfo *p = _info.servinfo; p != NULL; p = p->ai_next) {
        /* open a socket given servinfo */
        if ((socketfd = socket(p->ai_family,
                               p->ai_socktype,
                               p->ai_protocol)) == -1) {
            socketfd  = -1;
            continue;
        }
        /* assign port to socket */
        if (bind(socketfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(socketfd);
            socketfd = -1;
            continue;
        }
        /* if it gets to this point everything should be fine */
        _info.actual = p;
        break;
    }
    if (socketfd == -1) {
        printError("could not bind socket to any address");
        return -1;
    }
    if (listen(socketfd, LISTENER_BACKLOG) == -1) {
        return -1;
    }
    struct sockaddr_in *sockaddrin = (struct sockaddr_in *)(_info.actual->ai_addr);
    std::cout << "Server mounted succesfully on "
              << inet_ntoa(sockaddrin->sin_addr)
              << ":6667" << std::endl;
    _info.listener = socketfd;
    return _info.listener;
}

/* To be used on fatal errors only */
void Server::printError(string error) {
    std::cerr << "Server raised following error : " << error << std::endl;
}

// this might have to manage signals at some point ?? 
int Server::mainLoop(void) {

    _manager.setUpListener(_info.listener);

    while (42) {
        /* -1 = wait until some event happens */
        if (poll(_manager.fds, _manager.fds_size, -1) == -1)
            return -1;
        for (int fd_idx = 0; fd_idx < _manager.fds_size; fd_idx++) {
            if (_manager.hasDataToRead(fd_idx)) {
                /* listener is always at first entry */
                if (fd_idx == 0) {
                    _manager.addNewUser();
                    continue;
                } else {
                    char msg[100] = {0};
                    int bytes = recv(_manager.fds[fd_idx].fd, msg, sizeof(msg), 0);
                    if (bytes == -1) {
                        std::cerr << "bytes = -1" << std::endl;
                    } else if (bytes == 0) {
                        std::cout << "fd " << fd_idx << " closed connection" << std::endl;
                        close(_manager.fds[fd_idx].fd);
                        _manager.fds[fd_idx].fd = -1;
                    } else {
                        string message(msg, bytes);
                        std::cout << "fd : " << fd_idx << "Message : [" << message << "]" << std::endl;
                        //send(_manager.fds[fd_idx].fd, "001 ")
                    }
                }
                //else read/show in chat etc etc etc.
            } 
        }
    }
}


/* serverParams Section -------------------------- */

serverParams::serverParams(void)
    :
        servinfo(NULL),
        actual(NULL),
        listener(-1)
{}

serverParams::~serverParams(void) {
    if (listener != -1) {
        close(listener);
    }
}

/* serverFds Section -------------------------- */

serverFds::serverFds(void)
    :
        fds_size(0)
{}

serverFds::~serverFds(void) {
}

void serverFds::setUpListener(int listener) {
    fds[0].fd = listener;
    fds[0].events = POLLIN;
    fds[0].revents = 0;
    fds_size++;
}

int serverFds::hasDataToRead(int entry) {
    return fds[entry].revents & POLLIN;
}

int serverFds::addNewUser(void) {
    struct sockaddr_storage client;
    socklen_t addrlen = sizeof(struct sockaddr_storage);
    int fd_new = -1;
    if ((fd_new = accept(fds[0].fd, (struct sockaddr *)&client,
                         &addrlen)) == -1)
    {
        return -1;
    }
    fds_size++;

    fds[fds_size - 1].fd = fd_new;
    fds[fds_size - 1].events = POLLIN;
    fds[fds_size - 1].revents = 0;
    if (client.ss_family == AF_INET)  {
        char str[14];
        struct sockaddr_in *ptr = (struct sockaddr_in *)&client;
        inet_ntop(AF_INET, &(ptr->sin_addr), str, sizeof(str));
        string ip(str);
        std::cout << "connected to " << ip << std::endl;
    } else {
        char str[20];
        struct sockaddr_in *ptr = (struct sockaddr_in *)&client;
        inet_ntop(AF_INET, &(ptr->sin_addr), str, sizeof(str));
        string ip(str);
        std::cout << "connected to " << ip << std::endl;	
    }
    return 0;
}


} /* namespace irc */

