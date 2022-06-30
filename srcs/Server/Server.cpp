#include <iostream>
#include <utility>

#include <string.h>

#include "Server.hpp"
#include "User.hpp"
#include "Exceptions.hpp"
#include "Command.hpp"
#include "Tools.hpp"
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

Server::Server(void)
:
    fd_manager()
{
    if (setServerInfo() == -1
        || setListener() == -1)
    {
        throw irc::exc::ServerSetUpError();
    }
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
    memset(srv_buff, '\0', SERVER_BUFF_MAX_SIZE);
    srv_buff_size = 0;
    loadCommandMap();
    mainLoop();
}

Server::~Server(void) {
}

// this might have to manage signals at some point ?? 
int Server::mainLoop(void) {

    fd_manager.setUpListener();
    while (42) {
        fd_manager.Poll();
        for (int fd_idx = 0; fd_idx < fd_manager.fds_size; fd_idx++) {
            if (fd_manager.skipFd(fd_idx)) {
                continue;
            }
            /* this next if block seems useless. */
            /*if (fd_manager.hasHangUp(fd_idx)) {
                std::cout << "here" << std::endl;
                RemoveUser(fd_idx);
                fd_manager.CloseConnection(fd_idx);
                continue;
            }*/
            if (!fd_manager.hasDataToRead(fd_idx)) {
                continue;
            }
            /* listener is always at first entry */
            if (fd_idx == 0) {
                int new_fd = fd_manager.AcceptConnection();
                AddNewUser(new_fd);
                continue;
            }
            DataFromUser(fd_idx);
        }
    }
}

void Server::AddNewUser(int fd) {
    /* case server is full of users */
    if (fd == -1) {
        return ;
    }
    User user(fd);
    fd_user_map.insert(std::make_pair(fd, user));
}

void Server::RemoveUser(int fd_idx) {
    int fd = fd_manager.fds[fd_idx].fd;
    //std::cout << "User with fd : " << fd << " disconnected " << std::endl;
    FdUserMap::iterator it = fd_user_map.find(fd);
    User &user = it->second;
    /* If the user had a nick registered, erase it */
    if (nick_fd_map.count(user.nick)) {
        nick_fd_map.erase(user.nick);
    }
    /* erase user from fd map (this entry is created after connection) */
    fd_user_map.erase(fd);
}

/* 
 * Gestiona toda la logica para recoger el comando con buffering (guardar
 * leftovers, agregarlos al comando recibido, eliminarlos si excede la
 * longitud maxima, etc.)
 * El comando más largo que podrá tener en memoria el servidor será de
 * 512 * 2 bytes, que se corresponde con el caso en que un usuario envía
 * un primer comando incompleto (sin CRLF). En el caso en que un comando
 * sumado a los leftovers exceda los 512 bytes, se enviará ERR_INPUTTOOLONG 
 * y el comando será vaciado.
 * Cuando un comando más sus leftovers superan los 512 bytes y la string total
 * no contenga CRLF, se vaciará el comando y no se enviará nada.
 */
string Server::processCommandBuffer(int fd) {

    FdUserMap::iterator it = fd_user_map.find(fd);
    User &user = it->second;

    string cmd_string(srv_buff, srv_buff_size);
    tools::clean_buffer(srv_buff, srv_buff_size);
    srv_buff_size = 0;
    
    if (tools::ends_with(cmd_string, CRLF)) {
        /* add leftovers at start of buffer recieved */
        if (user.hasLeftovers()) {
            cmd_string.insert(0, user.BufferToString());
        }
        /* total buffer is too big */
        if (cmd_string.length() > SERVER_BUFF_MAX_SIZE) {
            user.resetBuffer();
            string reply(ERR_INPUTTOOLONG+user.nick+STR_INPUTTOOLONG);
            DataToUser(fd, reply);
            return "";
        }
    // cmd_string stays as it is.
    } else {
        size_t pos = cmd_string.find_last_of(CRLF);
        // no CRLF found
        if (pos == std::string::npos) {
            // ill-formated long comand
            if (cmd_string.length() + user.buffer_size > SERVER_BUFF_MAX_SIZE) {
                user.resetBuffer();
                return "";
            }
            /* buffer has space left : save and return empty command */
            user.addLeftovers(cmd_string);
            return "";
        }
        /* if CRLF is somewhere, construct comand until last CRLF
         * and save leftovers */
        string leftovers = cmd_string.substr(pos);
        user.addLeftovers(leftovers);
        cmd_string = cmd_string.substr(0, pos);
    }
    return cmd_string;
}

void Server::DataFromUser(int fd_idx) {

    int fd = fd_manager.fds[fd_idx].fd;
    srv_buff_size = recv(fd, srv_buff, sizeof(srv_buff), 0);

    if (srv_buff_size == -1) {
        throw irc::exc::FatalError("recv -1");
    }
    if (srv_buff_size == 0) {
        RemoveUser(fd_idx);
        fd_manager.CloseConnection(fd_idx);
        return ;
    }

    string cmd_string = processCommandBuffer(fd);

    /* cmd_string can be empty here in case there have been buffering 
     * problems with the user */
    if (cmd_string.empty()) {
        return ;
    }
    /* parse all commands */
    vector<string> cmd_vector;
    tools::split(cmd_vector, cmd_string, CRLF);
    int cmd_vector_size = cmd_vector.size();
    for (int i = 0; i < cmd_vector_size; i++) {
        Command command;
        if (command.Parse(cmd_vector[i]) != command.OK) {
            break;
        }
        /* command does not exist / ill formatted command */
        if (!cmd_map.count(command.Name())) {
            break;
        }
        CommandMap::iterator it = cmd_map.find(command.Name());
        (*this.*it->second)(command, fd);
    }
}

int Server::DataToUser(int fd, string &msg) {
    msg.insert(0, ":" + hostname);
    msg.insert(msg.size(), CRLF);
    //std::cout << "sending : [" << msg << "]" << std::endl;
    if (send(fd, msg.c_str(), msg.size(), 0) == -1) {
        throw irc::exc::FatalError("send = -1");
    }
    return 0;
}

} /* namespace irc */

