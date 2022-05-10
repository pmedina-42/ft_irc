#include <iostream>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/utsname.h>

#include "../includes/Server.hpp"

#define NAME_MAX_SZ 10

bool running = true;

using namespace irc;

/* -1 : error,
 * 0 : ok
 */
int getMyIP(void) {
    struct utsname buf;
    size_t node_len;
    struct hostent *hent;

    if (uname(&buf))
        return -1;

    node_len = strlen(buf.nodename) + 1;
    if (node_len > 255)
        return -1;
    hent = gethostbyname(buf.nodename);
    /* only ipv4 is accepted */
    if (hent->h_length != 4)
        return -1;
    char aux_out_str[16];
    int bytes_written = 0;

    /* Our own inet_ntoa (by carce-bo) */
    /* IP addresses are stored as 4 byte strings. */
    for (int j = 0; j < hent->h_length; j++) {
        if (j != 0) {
            *(aux_out_str + bytes_written++) = '.';
        }
        /* h_addr = h_addr_list[0] */
        int value = (int)(*(hent->h_addr + j));
        if (value >= 100) {
            *(aux_out_str + bytes_written++) = (value / 100) + '0';
            value %= 100;
            *(aux_out_str + bytes_written++) = (value / 10) + '0';
            value %= 10;
            *(aux_out_str + bytes_written++) = value + '0';
        } else if (value >= 10) {
            *(aux_out_str + bytes_written++) = (value / 10) +'0';
            value %= 10;
            *(aux_out_str + bytes_written++) = value + '0';
        } else {
            *(aux_out_str + bytes_written++) = value + '0';
        }
    }
    *(aux_out_str + bytes_written) = '\0';
    std::cout << "Connect to " << aux_out_str << std::endl;
    return 0;
}

void error(std::string msg) {
    std::cout << msg << std::endl;
    exit(1);
}

/* n = 1 : port
 * n = 2 : host addr
 * n = 3 : server password (?)
 */
int main(int argc, char **argv) {
    if (argc == 1) {
        Server server;
        return 42;
    } else {
        std::string hostname(argv[1]);
        std::string port(argv[2]);
        Server(hostname, port);
        // connect to server using params
        return 1;
    }
    /* ./server [port] */
    /*if (n != 2)
        error("bad arguments");
    Server server(atoi(v[1]));
    if (bind(server.fd, (struct sockaddr *)&server.addr, sizeof(server.addr)) == -1)
        error("bind error");
    server.getMyIp();
    if (listen(server.fd, INT_MAX) == -1) 
        error("listen error");
    while (running) {
        if (poll(&server.fds[0], server.fds.size(), -1) == -1)
            error("poll error");
        if (server.fds[0].revents == POLLIN) {
            server.addNewUser();
        } else {
            server.sendMessage();
        }
        if (!server.hasUsers()) { 
            running = false;
        }
    }*/
    return 0;
}
