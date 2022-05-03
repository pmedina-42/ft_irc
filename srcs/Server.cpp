#include "../includes/Server.hpp"
#include <netinet/in.h>
#include <sys/utsname.h>
#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

namespace irc {

Server::Server(void) {
	if (setServerInfo() == -1
		|| setListener() == -1)
	{
		// maybe throw ?
		exit(1);
	}
}

Server::~Server(void) {
}

int get_addrinfo_from_params(const char* hostname, const char *port,
							 struct addrinfo *hints,
							 struct addrinfo *servinfo)
{
	int ret = -1;

	if ((ret = getaddrinfo(hostname, port, hints, &servinfo)) != 0) {
		std::string error("getaddrinfo error :");
		std::cerr << error.append(gai_strerror(ret)) << std::endl;
		return -1;
	}
	/* Filter out IPv6 cases (makes me dizzy) */
	if (servinfo->ai_addrlen != 4) {
		std::cerr << "unsupported IP address length" << std::endl;
		return -1;
	}
	return 0;
}

/*
 * sets all entries in internal struct addrinfo. This includes
 * hostname, the list of addresses to be used, the IRC port,
 * the transport protocol and the IP protocol.
 * Since this function is
 * essential to the server functionality, shuts down the 
 * program in case of failure.
 * 
 * returns 0 on success, 1 otherwise. 
 */
int Server::setServerInfo(void) {
	struct addrinfo hints;
	struct addrinfo *servinfo;
	char hostname[96];
	size_t hostname_len = 96;
	const char* port = "6667";

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; // Ipv4
	hints.ai_socktype = SOCK_STREAM; // TCP 

	if (gethostname(&hostname[0], hostname_len) != 0) {
		printError("gethostname error");
		return -1;
	}
	if (get_addrinfo_from_params(hostname, port, &hints, servinfo) == -1) {
		if (servinfo != NULL) {
			freeaddrinfo(servinfo);
			return -1;
		}
	}
	/* This struct addrinfo may not be the one that binds, but
	 * thats one more step from here. */
	_info.servinfo = servinfo;
	return 0;
}

/*
 * Here ip works as the hostname. It is already null terminated
 * when calling std::string c_str method.
 */
int Server::setServerInfo(std::string &ip, std::string &port) {
	struct addrinfo hints;
	struct addrinfo *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // Ipv4
	hints.ai_socktype = SOCK_STREAM; // TCP 
	
	/* in_addr and sockaddr_in are both just uint32_t */
	//if (inet_aton(ip.c_str(), (in_addr *)hints.ai_addr) == 0)
	//	return -1;

	if (get_addrinfo_from_params(ip.c_str(), port.c_str(), &hints, servinfo) == -1) {
		if (servinfo != NULL) {
			freeaddrinfo(servinfo);
			return -1;
		}
	}
	/* This struct addrinfo may not be the one that binds, but
	 * thats one more step from here. */
	_info.servinfo = servinfo;
	return 0;
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
		return -1;
	}
	if (listen(socketfd, LISTENER_BACKLOG) == -1) {
		return -1;
	}
	_info.listener = socketfd;
	return _info.listener;
}

int Server::setListener(void) {
	
	int socketfd = -1;
	int yes = 1;

	if (socketfd = socket(AF_UNSPEC, SOCK_STREAM, 0) == -1)
		return -1;
	if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR,
				   &yes, sizeof(yes) == -1)
		/* assign port to socket */
		|| bind(socketfd, _info.servinfo->ai_addr, 
						  _info.servinfo->ai_addrlen) == -1)
	{
		if (socketfd != -1) {
			close(socketfd);
			return -1;
		}
	}
	if (listen(socketfd, LISTENER_BACKLOG) == -1)
		return -1;
	return socketfd;
}


/* To be used on fatal errors only */
void Server::printError(std::string error) {
	std::cerr << "Server raised following error : " << error << std::endl;
}


/* serverParams Section -------------------------- */

serverParams::serverParams(void)
	:
		servinfo(NULL),
		actual(NULL),
		listener(-1)
	{}

serverParams::~serverParams(void) {
	if (listener != -1) {
		close(listener);
	}
}

} /* namespace irc */

