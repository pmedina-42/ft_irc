#include <iostream>
#include <utility>

#include <string.h>

#include "Server.hpp"
#include "User.hpp"
#include "Exceptions.hpp"
#include "Command.hpp"
#include "Tools.hpp"

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
    User user = it->second;
    /* If the user had a nick registered, erase it */
    if (nick_fd_map.count(user.nick)) {
        nick_fd_map.erase(user.nick);
    }
    /* erase user from fd map (this entry is created after connection) */
    fd_user_map.erase(fd);
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
    string cmd_string(srv_buff, srv_buff_size);
    tools::clean_buffer(srv_buff, srv_buff_size);
    srv_buff_size = 0;

    
    /*
     * - First, check if it ENDS in CRLF.
     * - If it does not END in CRLF, but CONTAINS CRLF, theres new remains at end.
     * 
     * - IF it contains CRLF, check for remains and insert at beggining. 
     * - If it does not, save on remains.
     * 
     * 
     */
    // GESTIONAR BUFFER INTERNO DEL USER 

    /* Get current output + remains, then work with it. If command
     * + remains give ____ CRLF _____ CRLF ____, then execute first command, and save
     * second one. It is indeed a fucking mess but what else can I do.
    */

    /* parse all commands */
    vector<string> cmd_vector;
    tools::split(cmd_vector, cmd_string, CRLF);
    /* ignore empty commands */
    if (cmd_vector.empty()) {
        return ;
    }
    int cmd_vector_size = cmd_vector.size();
    for (int i = 0; i < cmd_vector_size; i++) {
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

