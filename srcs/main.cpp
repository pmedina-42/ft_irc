#include <iostream>
#include "../includes/Server.hpp"
#include <poll.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

#define NAME_MAX_SZ 10

bool running = true;

using namespace std;

void error(string msg) {
	cout << msg << endl;
	exit(1);
}

int main(int n, char **v) {
	/* ./server [port] */
	if (n != 2)
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
	}
}
