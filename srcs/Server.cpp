#include <sys/utsname.h>
#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <utility>
#include <fcntl.h>

#include "libft.h"

#include "Server.hpp"
#include "User.hpp"
#include "Exceptions.hpp"
#include "Command.hpp"
#include "Tools.hpp"
#include "Channel.hpp"
#include "NumericReplies.hpp"

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
 * 
 * Pointers to member functions :
 * https://cplusplus.com/forum/beginner/47833/
 * https://stackoverflow.com/questions/6265851/
 */

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

Server::Server(void)
:
    _info(),
    _fd_manager()
{
    if (setServerInfo() == -1
        || setListener() == -1)
    {
        throw irc::exc::ServerSetUpError();
    }
    struct sockaddr_in *sockaddrin = (struct sockaddr_in *)
                                        (_info.actual->ai_addr);
    hostname = (inet_ntoa(sockaddrin->sin_addr));
    memset(srv_buff, '\0', SERVER_BUFF_MAX_SIZE);
    srv_buff_size = 0;
    loadCommandMap();
    mainLoop();
}

Server::Server(string &hostname, string &port) {

    if (setServerInfo(hostname, port) != 0
        || setListener() == -1)
    {
        throw irc::exc::ServerSetUpError();
    }
    struct sockaddr_in *sockaddrin = (struct sockaddr_in *)
                                        (_info.actual->ai_addr);
    hostname = (inet_ntoa(sockaddrin->sin_addr));
    memset(srv_buff, '\0', SERVER_BUFF_MAX_SIZE);
    srv_buff_size = 0;
    loadCommandMap();
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
            close(socketfd);
            socketfd = -1;
            continue;
        }
        /* if it gets to this point everything should be fine */
        _info.actual = p;
        break;
    }
    if (socketfd == -1) {
        std::cerr << "could not bind socket to any address" << std::endl;
        return -1;
    }
    if (listen(socketfd, LISTENER_BACKLOG) == -1) {
        return -1;
    }
    struct sockaddr_in *sockaddrin = (struct sockaddr_in *)
                                      (_info.actual->ai_addr);
    hostname = inet_ntoa(sockaddrin->sin_addr);
    std::cout << "Server mounted succesfully on "
              << hostname
              << ":6667" << std::endl;
    _info.listener = socketfd;
    return _info.listener;
}

void Server::loadCommandMap(void) {
    cmd_map.insert(std::make_pair(string("NICK"), (&Server::NICK)));
    cmd_map.insert(std::make_pair(string("USER"), (&Server::USER)));
}


// this might have to manage signals at some point ?? 
int Server::mainLoop(void) {

    _fd_manager.setUpListener(_info.listener);
    while (42) {
        /* -1 = wait until some event happens */
        if (poll(_fd_manager.fds, _fd_manager.fds_size, -1) == -1)
            return -1;
        for (int fd_idx = 0; fd_idx < _fd_manager.fds_size; fd_idx++) {
            if (_fd_manager.hasHangUp(fd_idx) == true) {
                _fd_manager.CloseConnection(fd_idx);
                continue;
            }
            if (_fd_manager.hasDataToRead(fd_idx) == false) {
                continue;
            }
            /* listener is always at first entry */
            if (fd_idx == 0) {
                int new_fd = _fd_manager.AcceptConnection();
                AddNewUser(new_fd);
                continue;
            }
            int fd = _fd_manager.fds[fd_idx].fd;
            srv_buff_size = recv(fd, srv_buff, sizeof(srv_buff), 0);
            DataFromUser(fd, fd_idx);
        }
    }
}

void Server::AddNewUser(int fd) {
    User user(fd);
    fd_user_map.insert(std::make_pair(fd, user));
}

void Server::RemoveUser(int fd) {
    FdUserMap::iterator it = fd_user_map.find(fd);
    User user = it->second;
    /* If the user had a nick registered, erase it */
    if (nick_fd_map.count(user.nick)) {
        nick_fd_map.erase(user.nick);
    }
    /* erase user from fd map (this entry is created after connection) */
    fd_user_map.erase(fd);
}

void Server::DataFromUser(int fd, int fd_idx) {

    if (srv_buff_size == -1) {
        throw irc::exc::FatalError("recv -1");
    }
    if (srv_buff_size == 0) {
        _fd_manager.CloseConnection(fd_idx);
        RemoveUser(fd);
        return ;
    }
    string cmd_string(srv_buff, srv_buff_size);
    tools::clean_buffer(srv_buff, srv_buff_size);
    srv_buff_size = 0;
    // GESTIONAR BUFFER INTERNO DEL USER 

    /* Get current output + remains, then work with it. If command
     * + remains give ____ CRLF ___, then execute first command, and save
     * second one. It is indeed a fucking mess but what else can I do.
    */
    /* If the command is incomplete, save content on user
     * buffer */

    /* parse all commands */
    vector<string> cmd_vector;
    tools::split(cmd_vector, cmd_string, CRLF);
    /* ignore empty commands */
    if (cmd_vector.empty()) {
        return ;
    }
    int cmd_vector_size = cmd_vector.size();
    int ret = OK;
    for (int i = 0; i < cmd_vector_size && ret == OK; i++) {
        //std::cout << "cmd vector : [" << cmd_vector[i] << "]" << std::endl;
        Command command;
        if (command.Parse(cmd_vector[i]) != command.OK) {
            break;
        }
        /* command does not exist / ill formatted command */
        if (!cmd_map.count(command.Name())) {
            break;
        }
        CommandMap::iterator it = cmd_map.find(command.Name());
        std::cout << "fd : " << fd << std::endl;
        ret = (*this.*it->second)(command, fd);
    }
    if (ret == ERR_FATAL) {
        throw irc::exc::FatalError("command execution fatal error");
    }
}

int Server::DataToUser(int fd, string &msg) {
    msg.insert(0, ":" + hostname);
    msg.insert(msg.size(), CRLF);
    std::cout << "sending : [" << msg << "]" << std::endl;
    if (send(fd, msg.c_str(), msg.size(), 0) == -1) {
        return -1;
    }
    return 0;
}


/* AddressInfo Section -------------------------- */
AddressInfo::AddressInfo(void)
:
    servinfo(NULL),
    actual(NULL),
    listener(-1)
{}

AddressInfo::~AddressInfo(void) {
    if (listener != -1) {
        close(listener);
    }
}

/* FdManager Section -------------------------- */
FdManager::FdManager(void)
:
    fds_size(0)
{}

FdManager::~FdManager(void) {
}

void FdManager::setUpListener(int listener) {
    fds[0].fd = listener;
    fds[0].events = POLLIN;
    fds[0].revents = 0;
    fds_size++;
}

bool FdManager::hasDataToRead(int entry) {
    return (fds[entry].revents & POLLIN) ? true : false;
}

bool FdManager::hasHangUp(int entry) {
    return (fds[entry].revents & POLLHUP) ? true : false;
}

/* calls accept, and prepares the fd returned to be polled correctly. 
 * Throws in case of fatal error.
 */
int FdManager::AcceptConnection(void) {
    struct sockaddr_storage client;
    socklen_t addrlen = sizeof(struct sockaddr_storage);
    int fd_new = -1;
    /* get new fd from accepted connection */
    if ((fd_new = accept(fds[0].fd, (struct sockaddr *)&client,
                         &addrlen)) == -1)
    {
        throw irc::exc::FatalError("accept -1");
    }
    fds_size++;
    /* set to non blocking fd */
    if (fcntl(fd_new, F_SETFL, O_NONBLOCK) == -1) {
        throw irc::exc::FatalError("fctnl -1");
    }
    /* set up fd for poll */
    fds[fds_size - 1].fd = fd_new;
    fds[fds_size - 1].events = POLLIN;
    fds[fds_size - 1].events += POLLHUP;
    fds[fds_size - 1].revents = 0;

    /* debug information */
    if (client.ss_family == AF_INET)  {
        char str[14];
        struct sockaddr_in *ptr = (struct sockaddr_in *)&client;
        inet_ntop(AF_INET, &(ptr->sin_addr), str, sizeof(str));
        std::cout << "connected to " << str << std::endl;
    } else {
        char str[20];
        struct sockaddr_in6 *ptr = (struct sockaddr_in6 *)&client;
        inet_ntop(AF_INET6, &(ptr->sin6_addr), str, sizeof(str));
        std::cout << "connected to " << str << std::endl;
    }
    return fd_new;
}

/* Maybe eventually handle closing connections in such a way
 * we dont end up with all fd's at -1 ? */
void FdManager::CloseConnection(int fd_idx) {
    if (close(fds[fd_idx].fd) == -1) {
        throw irc::exc::FatalError("close -1");
    }
    fds[fd_idx].fd = -1;
}


} /* namespace irc */

