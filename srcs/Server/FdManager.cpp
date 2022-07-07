#include "Server.hpp"
#include "Exceptions.hpp"

#include <poll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <cerrno>

namespace irc {

/* Some socket errors, specially on send() should not terminate
 * the program. */
int FdManager::getSocketError(int fd) {

    int err_code;
    socklen_t len = sizeof(err_code);

    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err_code, &len) != 0) {
        err_code = errno;
    } else {
        errno = err_code;
    }
    return err_code;
}

FdManager::FdManager(void)
:
    fds_size(0)
{}

FdManager::~FdManager(void) {
    if (servinfo != NULL) {
        freeaddrinfo(servinfo);
    }
    for (int fd_idx = 0; fd_idx < fds_size; fd_idx++) {
        if (skipFd(fd_idx)) {
            continue;
        }
        if (close(fds[fd_idx].fd) == -1) {
            throw irc::exc::FatalError("close -1");
        }
    }
}

void FdManager::setUpListener(void) {
    fds[0].fd = listener;
    fds[0].events = POLLIN;
    fds[0].revents = 0;
    fds_size++;
}

void FdManager::Poll(void) {
    /* -1 = wait until some event happens */
    if (poll(fds, fds_size, -1) == -1) {
        throw irc::exc::FatalError("poll -1");
    }
}

bool FdManager::hasDataToRead(int entry) {
    return (fds[entry].revents & POLLIN) ? true : false;
}

bool FdManager::skipFd(int fd_idx) {
    return (fds[fd_idx].fd == -1);
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
    /* set to non blocking fd */
    if (fcntl(fd_new, F_SETFL, O_NONBLOCK) == -1) {
        throw irc::exc::FatalError("fctnl -1");
    }
    int fd_new_idx = -1;
    for (int i=0; i < fds_size; i++) {
        /* If there's a -1 somewhere, add new user there. */
        if (fds[i].fd == -1) {
            fd_new_idx = i;
            break;
        }
    }
    /* If all entries are occupied, increase number of fd's */
    if (fd_new_idx == -1) {
        /* case server is at full users */
        if (fds_size == MAX_FDS) {
            if (close(fd_new) == -1) {
                throw irc::exc::FatalError("close -1");
            }
            return -1;
        }
        /* else just increment the size, and add at new last position */
        fds_size++;
        fd_new_idx = fds_size - 1;
    }
    /* set up fd for poll */
    fds[fd_new_idx].fd = fd_new;
    fds[fd_new_idx].events = POLLIN;
    fds[fd_new_idx].revents = 0;

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
