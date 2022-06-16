#include <sys/utsname.h>
#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <utility>

#include "libft.h"

#include "Server.hpp"
#include "User.hpp"
#include "Exceptions.hpp"
#include "Command.hpp"
#include "Tools.hpp"
#include "Channel.hpp"

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
    memset(srv_buff, '\0', SERVER_BUFF_MAX_SIZE);
    srv_buff_size = 0;
    loadCommandMap();
    loadNickVector();
    mainLoop();
}

Server::Server(string &hostname, string &port) {

    if (setServerInfo(hostname, port) != 0
        || setListener() == -1)
    {
        throw irc::exc::ServerSetUpError();
    }
    memset(srv_buff, '\0', SERVER_BUFF_MAX_SIZE);
    srv_buff_size = 0;
    loadCommandMap();
    loadNickVector();
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
    std::cout << "Server mounted succesfully on "
              << inet_ntoa(sockaddrin->sin_addr)
              << ":6667" << std::endl;
    _info.listener = socketfd;
    return _info.listener;
}

void Server::loadCommandMap(void) {
    cmd_map.insert(std::make_pair(string("NICK"), (&Server::NICK)));
    cmd_map.insert(std::make_pair(string("USER"), (&Server::USER)));
}

void Server::loadNickVector(void) {
    nick_vector.reserve(MAX_FDS);
    for (int i=0; i < MAX_FDS; i++) {
        nick_vector.push_back("");
    }
}


// this might have to manage signals at some point ?? 
int Server::mainLoop(void) {

    _fd_manager.setUpListener(_info.listener);
    while (42) {
        /* -1 = wait until some event happens */
        if (poll(_fd_manager.fds, _fd_manager.fds_size, -1) == -1)
            return -1;
        for (int fd_idx = 0; fd_idx < _fd_manager.fds_size; fd_idx++) {
            if (_fd_manager.hasDataToRead(fd_idx) == false) {
                continue;
            }
            /* listener is always at first entry */
            if (fd_idx == 0) {
                _fd_manager.addNewUser();
                continue;
            }
            // 1. check message format is correct
            // 2. parse command & params
            // 3. check user can indeed call command (blacklist, is chanop, etc)
            // 4. prepare command response.
            // 5. exec/send command response.
            srv_buff_size = recv(_fd_manager.fds[fd_idx].fd, srv_buff,
                             sizeof(srv_buff), 0);
            if (srv_buff_size == -1) {
                string exception_msg("recv = -1");
                throw exc::FatalError(exception_msg);
            }
            if (srv_buff_size == 0) {
                // MANAGE CLOSED CONNECTION
                close(_fd_manager.fds[fd_idx].fd);
                _fd_manager.fds[fd_idx].fd = -1;
                continue;
            }
            MessageFromUser(fd_idx);
            tools::clean_buffer(srv_buff, srv_buff_size);
            /*string message(msg, bytes);
            char **arr = ft_split(message.c_str(), ' ');
            if (arr == NULL)
                std::cerr << "XDDD " << std::endl;
             //ft_strlen() - 1 intenta no llevarse el \n del final !! Esto hace
               // * que el cliente parsee los mensajes de uno en uno y dan cosas raras.
               // * de esta forma, no sucede 
            for (int i=0; arr[i] != NULL; i++) {
                std::string word(arr[i]);
                if (word.compare("NICK") == 0) {
                    nickname = string(arr[i + 1], ft_strlen(arr[i + 1]) - 1);
                    size_t carriage_return_pos = nickname.find('\r');
                    if (carriage_return_pos != string::npos) {
                        std::cout << "b" << std::endl;
                        nickname = nickname.substr(0, carriage_return_pos);
                    }
                } else if (word.compare("USER") == 0) {
                    std::cout << "a" << std::endl;
                    username = string(arr[i + 1]);
                } else if (word.compare("JOIN") == 0) {
                    char prefix = arr[1][0];
                    string name = arr[1]++;
                    std::cout << "nuevo canal que se tiene que crear con usuario a saber, prefijo " << prefix << " y nombre " << name << std::endl; 
                    //Channel* channel = new Channel(prefix, name, );
                    //_channels.push_back(channel);
                } else if (word[0] == ':') {
                    // -2 por la misma razon que strlen - 1
                    realname = word.substr(1, word.size() - 2);
                    break;
                }
            }
            for (int i=0; arr[i] != NULL; i++) {
                free(arr[i]);
            }
            free(arr);
            //send(_fd_manager.fds[fd_idx].fd, "001 ")
            }
        }
        if (!nickname.empty() && !username.empty()) {
            //_users.push_back(new irc::User())
            std::cout << "about to send repl y " << std::endl;
            struct sockaddr_in *sockaddrin = (struct sockaddr_in *)(_info.actual->ai_addr);
            string our_host(inet_ntoa(sockaddrin->sin_addr));*/
            // pon aqui tus dos credenciales, es nicknaem!username@hostname o ip
            // https://stackoverflow.com/questions/662918/how-do-i-concatenate-multiple-c-strings-on-one-line
            /* Fallaba esto (RFC 2812 section 2.4 (page 7)) :
             * Most of the messages sent to the server generate a reply of some
             * sort.  The most common reply is the numeric reply, used for both
             * errors and normal replies.  The numeric reply MUST be sent as one
             * message consisting of the sender prefix, the three-digit numeric, and
             * the target of the reply.
             * Vale: resulta que weechat está programado en windows o para windows o de alguna
             * forma relacionado con windows de manera que está introduciendo saltos de linea como
             * \r\n en vez de \n. Este \r lo que hace es retornar el carro y reescribirme la string
             * cuando la imprimo por pantalla haciendo que aparezcan cosas super raras.
             */
            /*std::string sender_prefix = ":" + our_host;
            std::string rpl_welcome("001");
            std::string target_of_reply = username;
            std::string complete_name = nickname + "!" + username + "@" + "192.168.45.85";
            std::cout << "our_host : [" << our_host << "], target_of_reply : [" << target_of_reply << "], complete_name : [" << complete_name << "]" << std::endl;
             IRC USES CRLF !! specified in RFC 1459
            string reply = sender_prefix + " " + rpl_welcome + " " + target_of_reply + " :Welcome to wassap 2 " + complete_name + "\r\n";
            std::cout << "reply : [" << reply << "]" << std::endl;
            send(_fd_manager.fds[1].fd, reply.c_str(), reply.length(), 0);
            send(_fd_manager.fds[2].fd, reply.c_str(), reply.length(), 0);*/
        }
    }
}

int Server::MessageFromUser(int fd_idx) {

    string cmd_string(srv_buff, srv_buff_size);
    /* reset server buff */
    tools::clean_buffer(srv_buff, srv_buff_size);
    srv_buff_size = 0;

    /* parse all commands */
    vector<string> cmd_vector;
    tools::split(cmd_vector, cmd_string, CRLF);
    /* ignore empty commands */
    if (cmd_vector.empty()) {
        return CLEAN;
    }

    int cmd_vector_size = cmd_vector.size();
    for (int i = 0; i < cmd_vector_size; i++) {
        //std::cout << "cmd_vector[" << i << "] : [" << cmd_vector[i] << "] " << std::endl;
        Command command;
        if (command.Parse(cmd_vector[i]) != command.OK) {
            break;
        }
        /* command does not exist / ill formatted command */
        if (!cmd_map.count(command.Name())) {
            break;
        }
        CommandMap::iterator it = cmd_map.find(command.Name());
        int ret = (*this.*it->second)(command, fd_idx); // XD
        if (ret == DONE) {
            continue;
        }
        // Aquí viene la matriz de comandos.
    }
    return CLEAN;
}

/**
 * Command: NICK
 * Parameters: <nickname>
 */
int Server::NICK(Command &cmd, int fd) {

    int size = cmd.args.size();
    if (size != 2) {
        return ERR_NO_REPLY;
    }
    string nick = cmd.Name();
    User new_user(fd, nick);   

    if (user_map.empty() || user_map.count(nick)) {
        user_map.insert(std::make_pair(nick, new_user));
        return DONE;
    }
    nick_vector[fd] = nick;
    /* molaría aquí construir el reply de error,
     * y enviarlo al correspondiente fd.
     */
    return SEND_ERR_REPLY;
}


/**
 * Command: USER
 * Parameters: <username> 0 * <realname>
 */
int Server::USER(Command &cmd, int fd) {

    int size = cmd.args.size();
    if (size != 5) {
        return ERR_NO_REPLY;
    }
    string nick = nick_vector[fd];
    if (!user_map.count(nick)) {
        // needs nick first, maybe it has an error message here.
        return ERR_NO_REPLY;
    }
    UserMap::iterator it = user_map.find(nick);
    User user = it->second;

    user.name = cmd.args[1];
    user.full_name = cmd.args[size - 1];

    return DONE;
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

int FdManager::addNewUser(void) {
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

