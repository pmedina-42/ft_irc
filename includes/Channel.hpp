#ifndef CHANNEL_H
#define CHANNEL_H

#include "User.hpp"
#include <list>

/* TODO: comprobar y setear tamaño máximo de usuarios dentro de un canal? */

class Channel {
	public:
		Channel(std::string, User*);
		~Channel();

		/* User list getters */
		inline std::list<User*> getUsers() { return _users; }
		inline std::list<User*> getOperators() { return _oper_users; }
		inline std::list<User*> getInvited() { return _invited_users; }
		inline std::list<std::string> getBlackList() { return _blackList; }
	
		/* Channel getters & setters */
		inline std::string getName() { return _name; }
		inline void setName(std::string name) { _name = name; }

		inline std::string getTopic() { return _topic; }
		inline void setTopic(std::string topic) { _topic = topic; }

		inline char getMode() { return _mode; }
		inline void setMode(char mode) { _mode = mode; }
	
		inline unsigned int getMaxUsers() { return _max_users; }
		inline void setMaxUsers(unsigned int max) { _max_users = max; }

		inline std::string getKey() { return _key; }
		inline void setKey(std::string key) { _key = key; }

		/* Class functions */

		void addUser(User*);
		void deleteUser(std::string);
		void banUser(std::string);
		bool userInBlackList(std::string);
		/* Esta función devuelve true si el canal está en modo invitación */
		bool inviteMode();
		/* Esta función devuelve true si un usuario está invitado al canal
		 * si devuelve true, el usuario puede unirse al canal, si no no */
		bool isInvited(std::string);
		void setUserMode(std::string, char);

	private:
		/* Lista de usuarios que pertenecen al canal */
		std::list<User*> _users;
		/* Lista de usuarios que son operadores */
		std::list<User*> _oper_users;
		/* Lista de usuarios invitados al canal */
		std::list<User*> _invited_users;
		/* Lista de nombres de usuarios baneados */
		std::list<std::string> _blackList;

		/* Nombre del canal */
		std::string _name;
		/* Tema del canal */
		std::string _topic;
		/* Modo del canal */
		char _mode;
		/* Maximo de usuarios en el canal */
		unsigned int _max_users;
		/* Clave, si es que la tiene y el modo es k */
		std::string _key;

};

#endif
