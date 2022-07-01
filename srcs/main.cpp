#include <iostream>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/utsname.h>

#include "Server.hpp"

#define NAME_MAX_SZ 10

/**
 * https://modern.ircdocs.horse/ for numeric replies
 * 
 * To anyone wondering how exceptions and memory deallocation on destructors
 * work :
 * Yes, destructors are guaranteed to be called on stack unwinding,
 * including unwinding due to exception being thrown.
 * There are only few tricks with exceptions that you have to remember:
 * Destructor of the class is not called if exception is thrown in its
 * constructor.
 *
 * NEXT THING : PING PONG LOOP
 */

bool running = true;

using namespace irc;

void error(std::string msg) {
    std::cout << msg << std::endl;
    exit(1);
}

/* n = 1 : port
 * n = 2 : host addr
 * n = 3 : server password (?) // no server password 
 */
int main(int argc, char **argv) {
    if (argc == 1) {
        Server server;
        return 42;
    } else if (argc == 3) {
        std::string hostname(argv[1]);
        std::string port(argv[2]);
        Server(hostname, port);
        // connect to server using params
        return 1;
    } else {
        return 42;
    }
}
