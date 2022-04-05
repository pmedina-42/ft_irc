#ifndef CHANNEL_H
#define CHANNEL_H

#include "User.hpp"
#include <list>

class Channel {
	public:
		/* Un canal se inicializa con un nombre y el usuario que lo crea */
		/* Este usuario recibe automaticamente el rol de operador */
		Channel(std::string, User*);
		/* Un canal se destruye cuando ya no quedan usuarios */
		/* En principio no habría que hacer nada en el destructor */
		~Channel();

		/* Lo suyo sería una función para añadir usuarios al canal y otra 
		 * para borrar. Al un usuario intentar */
		
		/* Getters & Setters */
		std::list<User*> getUsers();
		std::list<User*> getOperators();
		std::list<User*> getInvited();
		std::list<std::string> getBanned();
		std::string getName();
		void setName(std::string);
		std::string getTopic();
		void setTopic(std::string);
		char getMode();
		void setMode(char);
		unsigned int getMaxUsers();
		void setMaxUsers(unsigned int);
		std::string getKey();
		void setKey(std::string);

	private:
		/* Lista de usuarios que pertenecen al canal */
		std::list<User*> _users;
		/* Lista de usuarios que son operadores */
		std::list<User*> _oper_users;
		/* Lista de usuarios invitados al canal */
		std::list<User*> _invited_users;
		/* Lista de nombres de usuarios baneados */
		std::list<std::string> _banned_users;

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

/* User list getters */
inline std::list<User*> Channel::getUsers() { return _users; }
inline std::list<User*> Channel::getOperators() { return _oper_users; }
inline std::list<User*> Channel::getInvited() { return _invited_users; }
inline std::list<std::string> Channel::getBanned() { return _banned_users; }

/* Channel getters & setters */
inline std::string Channel::getName() { return _name; }
inline void Channel::setName(std::string name) { _name = name; }

inline std::string Channel::getTopic() { return _topic; }
inline void Channel::setTopic(std::string topic) { _topic = topic; }

inline char  Channel::getMode() { return _mode; }
inline void Channel::setMode(char mode) { _mode = mode; }

inline unsigned int Channel::getMaxUsers() { return _max_users; }
inline void Channel::setMaxUsers(unsigned int max) { _max_users = max; }

inline std::string Channel::getKey() { return _key; }
inline void Channel::setKey(std::string key) { _key = key; }

#endif
