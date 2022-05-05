#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <iostream>
#include <string>

bool running = true;

using namespace std;

void error(std::string msg) {
	cout << msg << endl;
	_exit(1);
}

int main(int n, char **v) {
	/* ./client [port] [username] */
	if (n != 3)
		error("bad arguments");
	/* Se crea el socket. AF_INET = IPv4, SOCK_STREAM = TCP/IP,
	 * el 0 viene con el protocolo de transmision */
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	/* Esta estructura contiene toda la info del socket.
	 * Mas info preguntar a pmedina- */
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(v[1]));
	addr.sin_addr.s_addr = INADDR_ANY;
	/* Establece una conexion con otro socket
	 * que este corriendo en el mismo puerto */
	connect(fd, (struct sockaddr *)&addr, sizeof(addr));
	/* Una vez establecida la conexion le mando al servidor
	 * el nickname del cliente que se ha conectado */
	send(fd, v[2], strlen(v[2]), 0);
	/* Cojo el pid del padre para matarlo luego desde el
	 * hijo a la que reciba "exit" */
	pid_t ppid = getpid();
	pid_t pid = fork();
	if (pid == -1) {
		error("fork error");
	}
	if (pid == 0) {
		/* Mientras el hijo este corriendo va a esperar a recibir
		 * un input para enviarselo al servidor*/
		while (running) {
			std::string str;
			getline(cin, str);
			if (!str.compare("exit")) {
				running = false;
			}
			/* Le envio al servidor el input que ha metido el cliente.
			 * Al ser funcion de c hay que pasar la string a char* */
			send(fd, str.c_str(), str.length(), 0);
		}
		/* Cuando el cliente sale del chat el hijo mata al padre, aunque
		 * imprime cosa fea por pantalla y con hilos no, pero lo quería
		 * hacer con procesos */
		kill(ppid, SIGKILL);
	} else {
		/* Mientras el padre esta corriendo esta pendiente de recibir un
		 * mensaje que haya enviado otro cliente al servidor */
		while (running) {
			char serveroutput[256];
			/* Si lo recibe, rellena el char[] */
			recv(fd, &serveroutput, sizeof(serveroutput), 0);
			/* Lo saca por pantalla */
			cout << serveroutput;
			/* Y se vacía para cuando llegue el siguiente mensaje */
			memset(serveroutput, '\0', 255); 
		}
	}
	/* Se cierra el fd del socket */
	close(fd);
	return 0;
}
