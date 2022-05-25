#ifndef IRC42_TYPES_H
#define IRC42_TYPES_H

/* Archivo para meter enums, typedefs y mierdas que puedan ser necesarias */

#include <unordered_map>

using std::string;

class Channel;
class User;
class ChannelUser; // cambiar al nombre que haga falta cuando sea.

namespace irc {

    typedef std::unordered_map<string, Channel> channel_map;
    typedef std::unordered_map<string, User> user_map;
    typedef std::unordered_map<string, ChannelUser> channel_user_map;

}




#endif