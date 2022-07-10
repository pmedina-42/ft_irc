#include <iostream>
#include <utility>

#include <string.h>
#include <errno.h>
#include <time.h>

#include "Server.hpp"
#include "User.hpp"
#include "Exceptions.hpp"
#include "Command.hpp"
#include "Tools.hpp"
#include "NumericReplies.hpp"
#include "Log.hpp"

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
    start = time(NULL);
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
    start = time(NULL);
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

void Server::pongLoop(void) {
    for (int fd_idx = 0; fd_idx < fd_manager.fds_size; fd_idx++) {
        if (fd_manager.skipFd(fd_idx)) {
            continue;
        }
        int fd = fd_manager.getFdFromIndex(fd_idx);
        User &user = getUserFromFd(fd);
        (void)user;
    }

}

void Server::AddNewUser(int new_fd) {
    /* case server is full of users */
    if (new_fd == -1) {
        return ;
    }
    User user(new_fd);
    fd_user_map.insert(std::make_pair(new_fd, user));
}

void Server::RemoveUser(int fd_idx) {
    int fd = fd_manager.getFdFromIndex(fd_idx);
    FdUserMap::iterator it = fd_user_map.find(fd);
    User &user = it->second;

    LOG(INFO) << "User with nick [" << user.nick << "] removed";
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
            user.resetBuffer();
        }
        /* total buffer is too big */
        if (cmd_string.length() > SERVER_BUFF_MAX_SIZE) {
            string reply(ERR_INPUTTOOLONG+user.nick+STR_INPUTTOOLONG);
            LOG(WARNING) << "Buffer from User [" << user.nick << "] too long";
            DataToUser(fd, reply);
            return "";
        }
    // cmd_string stays as it is.
    } else {
        size_t pos = tools::find_last_CRLF(cmd_string);
        // no CRLF found
        if (pos == std::string::npos) {
            // ill-formated long comand
            if (cmd_string.length() + user.buffer_size > SERVER_BUFF_MAX_SIZE) {
                user.resetBuffer();
                LOG(WARNING) << "Ill formatted buffer from User [" << user.nick << "]";
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

    int fd = fd_manager.getFdFromIndex(fd_idx);
    srv_buff_size = recv(fd, srv_buff, sizeof(srv_buff), 0);

    if (srv_buff_size == -1) {
        throw irc::exc::FatalError("recv -1");
    }
    if (srv_buff_size == 0) {
        RemoveUser(fd_idx);
        fd_manager.CloseConnection(fd_idx);
        return ;
    }
    LOG(INFO) << "DataFromUser with fd " << fd
              << ", bytes : " << srv_buff_size;
    string cmd_string = processCommandBuffer(fd);

    //std::cout << "cmd string : [" << cmd_string << "]" <<  std::endl;   
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
            continue ;
        }
        /* command does not exist / ill formatted command */
        if (!cmd_map.count(command.Name())) {
            string msg(ERR_UNKNOWNCOMMAND+command.Name()+STR_UNKNOWNCOMMAND);
            DataToUser(fd_idx, msg);
            continue ;
        }
        CommandMap::iterator it = cmd_map.find(command.Name());
        (*this.*it->second)(command, fd_idx);
    }
}

/* 
 * sends [:<hostname> <msg>CRLF] to user with fd asociated.
 * 
 * Why send() function is controlled as follows : 
 * https://stackoverflow.com/questions/33053507/econnreset-in-send-linux-c
 * This way b_sent = 0 does not have to be controlled, because ECONNRESET
 * will be returned by send in case we try to send to a closed connection
 * twice.
 */
void Server::DataToUser(int fd_idx, string &msg) {

    msg.insert(0, ":" + hostname);
    msg.insert(msg.size(), CRLF);

    int fd = fd_manager.getFdFromIndex(fd_idx);
    int b_sent = 0;
    int total_b_sent = 0;

    do {
        b_sent = send(fd, &msg[b_sent], msg.size() - total_b_sent, 0);
        if (b_sent == -1) {
            int error = fd_manager.getSocketError(fd);
            if (error == ECONNRESET
                || error == EPIPE)
            {
                RemoveUser(fd_idx);
                fd_manager.CloseConnection(fd_idx);
                return ;
            }
            throw irc::exc::FatalError("send = -1");
        }
        /* add bytes sent to total */
        total_b_sent += b_sent;
    /* keep looping until full message is sent */
    } while (total_b_sent != (int)msg.size());

    return ;
}

User& Server::getUserFromFd(int fd) {
    FdUserMap::iterator it = fd_user_map.find(fd);
    return it->second;
}

} /* namespace irc */

