#include "Server.hpp"
#include "Exceptions.hpp"

#include <poll.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include <iostream>

namespace irc {

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

} //namespace