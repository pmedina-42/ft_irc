#include "Server.hpp"
#include "Exceptions.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#include <iostream>

namespace irc {

static int get_addrinfo_from_params(const char* hostname, const char *port,
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
        std::cerr << "gethostname error" << std::endl;
        return -1;
    }
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
    fd_manager.servinfo = servinfo;
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
    fd_manager.servinfo = servinfo;
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
    for (struct addrinfo *p = fd_manager.servinfo; p != NULL; p = p->ai_next) {
        /* open a socket given servinfo */
        if ((socketfd = socket(p->ai_family,
                               p->ai_socktype,
                               p->ai_protocol)) == -1)
        {
            socketfd  = -1;
            continue;
        }
        int yes = 1;
	    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &yes, sizeof(yes));
        /* assign port to socket */
        if (bind(socketfd, p->ai_addr, p->ai_addrlen) == -1) {
            if (close(socketfd) == -1) {
                freeaddrinfo(fd_manager.servinfo);
                return -1;
            }
            socketfd = -1;
            continue;
        }
        /* if it gets to this point everything should be fine */
        fd_manager.servinfo = p;
        break;
    }
    if (socketfd == -1) {
        std::cerr << "could not bind socket to any address" << std::endl;
        freeaddrinfo(fd_manager.servinfo);
        return -1;
    }
    if (listen(socketfd, LISTENER_BACKLOG) == -1) {
        freeaddrinfo(fd_manager.servinfo);
        return -1;
    }
    struct sockaddr_in *sockaddrin = (struct sockaddr_in *)
                                      (fd_manager.servinfo->ai_addr);
    hostname = inet_ntoa(sockaddrin->sin_addr);
    /* debug, might not need it in the end */
    std::cout << "Server mounted succesfully on " << hostname
              << ":6667" << std::endl;
    fd_manager.listener = socketfd;
    return socketfd;
}

void Server::loadCommandMap(void) {
    cmd_map.insert(std::make_pair(string("NICK"), (&Server::NICK)));
    cmd_map.insert(std::make_pair(string("USER"), (&Server::USER)));
}

}