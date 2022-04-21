#include "../includes/Server.hpp"
#include <netinet/in.h>
#include <sys/utsname.h>
#include <iostream>
#include <netdb.h>
#include <sys/socket.h>

using namespace std;

Server::Server(int port) {
	int opt = 1;
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;
	fds.push_back(pollfd());
	fds.back().fd = fd;
	fds.back().events = POLLIN;
	this->fd = fd;
}

void Server::addNewUser() {
	int sock = accept(fd, 0, 0);
	if (!sock) {
		cout << "accept error" << endl;
		exit(1);
	}
	char name[20];
	while (strncmp(name, "NICK", 4))
		recv(sock, &name, sizeof(name), 0);
	User *newUser = new User(sock, name+5, NAME_MAX_SZ);
	users.push_back(newUser);
	cout << "new connection: " << newUser->getNickName() << endl;
	 memset(name, '\0', 9);
	fds.push_back(pollfd());
	fds.back().fd = sock;
	fds.back().events = POLLIN;
}

void Server::sendMessage() {
	for (vector<pollfd>::iterator it = fds.begin(); it != fds.end(); it++) {
		if ((*it).revents == POLLIN) {
			size_t index;
		 	for (size_t i = 0; i != getUsers().size(); i++) {
		 		if (getUsers()[i]->getFd() == (*it).fd)
		 			index = i;
		 		}
		 	char input[256];
		 	recv((*it).fd, &input, sizeof(input), 0);
			cout << "input: " << input << endl;
		 	string output;
		 	if (!strcmp(input, "exit")) {
		 		getUsers()[index]->leave();
				output = getUsers()[index]->getNickName() + " has left the chat\n";
			} else {
				output = getUsers()[index]->getNickName() + ": " + input + "\n";
			}
			for (size_t i = 0; i != getUsers().size(); i++) {
				if (i != index) {
					cout << "output: " << output << endl;
					send(getUsers()[i]->getFd(), output.c_str(), output.length(), 0);
				}
			}
			/* Pone el input a 0 para recibir el siguiente */
			memset(input, '\0', 255);
		}
	}
}

/* -1 : error,
 * 0 : ok */
int Server::getMyIp() {
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
	cout << "Connect to " << aux_out_str << endl;
	return 0;
}

/* Recorre el vector<Users *> y a la que pilla uno que sigue perteneciendo al
* servidor devuelve true para que siga corriendo
*/
bool Server::hasUsers() {
	for (size_t i = 0; i != getUsers().size(); i++) {
		if (getUsers()[i]->stillThere())
			return true;
	}
	return false;
}
