#ifndef SERVER_H
#define SERVER_H

#define NAME_MAX_SZ 10

#include "User.hpp"
#include <vector>
#include <unistd.h>
#include <netinet/in.h>
#include <poll.h>

class User;

class Server {
	public:
		Server(int);
		~Server() {
			for (size_t i = 0; i != fds.size(); i++)
				close(fds[i].fd);
		}

		std::vector<User*> getUsers() { return users; }
		std::vector<pollfd> getFds() { return fds; }
		struct sockaddr_in* getAddr() { return &addr; }

		int getMyIp();
		bool hasUsers();
		void addNewUser();
		void sendMessage();

		struct sockaddr_in addr;
		std::vector<User *> users;
		std::vector<pollfd> fds;
		int fd;
};

#endif
