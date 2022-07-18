#ifndef IRC42_FDMANAGER_H
# define IRC42_FDMANAGER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <poll.h>

#include <string>
#include "Types.hpp"

namespace irc {

class FdManager {
    public:
    FdManager(void);
    FdManager(std::string &ip, std::string &port);
    FdManager(const FdManager &other);
    ~FdManager();

    /* Setup */
    int setUpAddress(void);
    int setUpAddress(std::string &hostname, std::string &port);
    int setUpListener(void);
    void setUpPoll(void);

    /* main utils */
    void Poll(void);
    int acceptConnection(void);
    void closeConnection(int fd_idx);

    /* accessors */
    bool hasDataToRead(int entry);
    bool skipFd(int fd_idx);
    int getFdFromIndex(int fd_idx);

    /* socket error helpers */
    bool socketErrorIsNotFatal(int fd);
    int getSocketError(int);
    /* fd from clients manager. This includes
    * the listener, at entry 0.
    */
    struct pollfd fds[MAX_FDS];
    int fds_size;
    struct addrinfo *servinfo;
    int listener;
    std::string hostname;
};

}

#endif /* IRC42_FDMANAGER_H */
