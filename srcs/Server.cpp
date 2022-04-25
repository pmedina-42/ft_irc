#include "../includes/Server.hpp"
#include <netinet/in.h>
#include <sys/utsname.h>
#include <iostream>
#include <netdb.h>
#include <sys/socket.h>

namespace irc {

Server::Server(void) {
	if (setServerInfo() != 0
		|| setListener() != 0)
	{
		// maybe throw ?
		exit(1);
	}
}

Server::~Server(void) {
}

int Server::setServerInfo(void) {
	struct addrinfo hints;
	struct addrinfo *servinfo;
	char hostname[96];
	size_t hostname_len = 96;
	const char* port = "6667";
	int ret = -1;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // Ipv4
	hints.ai_socktype = SOCK_STREAM; // TCP 

	if (gethostname(&hostname[0], hostname_len) != 0) {
		printError("gethostname error");
		return 1;
	}
	if ((ret = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
		std::string error("getaddrinfo error :");
		printError(error.append(gai_strerror(ret)));
		return 1;
	}
	/* Filter out IPv6 cases (makes me dizzy) */
	if (servinfo->ai_addrlen != 4) {
		printError("unsupported IP address length");
		return 1;
	}
	/* This struct addrinfo may not be the one that binds, but
	 * thats one more step from here. */
	_info.servinfo = servinfo;
	return 0;
}

int Server::setServerInfo(std::string &ip, std::string &port) {
	
}


int Server::setListener(void) {

	int socketfd = -1;
	
	/* loop through addresses until bind works */
    for (struct addrinfo *p = _info.servinfo; p != NULL; p = p->ai_next) {
		/* open a socket given servinfo */
		if ((socketfd = socket(p->ai_family,
							   p->ai_socktype,
							   p->ai_protocol)) == -1) {
			socketfd  = -1;
			continue;
		}
		int yes = 1;
		/* avoid "address already in use" error */
		if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR,
					   &yes, sizeof(yes) == -1)
			/* assign port to socket */
			|| bind(socketfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(socketfd);
			socketfd = -1;
			continue;
		}
		/* if it gets to this point everything should be fine */
		_info.actual = p;
		break;
	}
	if (socketfd == -1) {
		printError("could not bind socket to any address");
		return 1;
	}
	if (listen(socketfd, LISTENER_BACKLOG) == -1) {
		printError("listen error");
		return 1;
	}
	_info.listener = socketfd;
	return 0;
}


/* To be used on fatal errors only */
void Server::printError(std::string error) {
	std::cerr << "Server raised following error : " << error << std::endl;
}

} /* namespace irc */

