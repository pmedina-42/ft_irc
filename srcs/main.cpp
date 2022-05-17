#include <iostream>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/utsname.h>

#include "../includes/Server.hpp"

#define NAME_MAX_SZ 10

/**
 * See https://modern.ircdocs.horse/ for numeric replies
 * */

bool running = true;

using namespace irc;

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
}
