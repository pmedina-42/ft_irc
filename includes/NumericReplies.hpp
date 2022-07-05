#ifndef IRC42_NUMERICREPLIES_H
# define IRC42_NUMERICREPLIES_H

/* RFC 2812 section 2.4 (page 7) :
 * The numeric reply MUST be sent as one message consisting of
 * the sender prefix, the three-digit numeric, and the target of
 * the reply.
 */

/**
 * The first message sent after client registration
 */
# define RPL_WELCOME " 001 "
# define RPL_WELCOME_STR_1 " :Welcome to the Internet Relay Network, "

# define ERR_NONICKNAMEGIVEN " 431 "
# define STR_NONICKNAMEGIVEN " :No nickname given"

/**
 * The desired nickname contains characters that are disallowed by the server
 */
# define ERR_ERRONEUSNICKNAME " 432 "
# define STR_ERRONEUSNICKNAME " :Erroneus nickname"

/**
 * The desired nickname is already in use on the network
 */
# define ERR_NICKNAMEINUSE " 433 "
# define STR_NICKNAMEINUSE " :Nickname is already in use"

/**
 * Not enough parameters were supplied
 */
# define ERR_NEEDMOREPARAMS " 461 "
# define STR_NEEDMOREPARAMS " :Not enough parameters"

/**
 * Client tries to change a detail that can only be set during registration
 */
# define ERR_ALREADYREGISTERED " 462 "
# define STR_ALREADYREGISTERED " :You may not reregister"

/**
 * JOIN failed because the client has joined their maximum number of channels
 */
# define ERR_TOOMANYCHANNELS " 405 "
# define STR_TOOMANYCHANNELS " :You have joined too many channels"

/**
 * Returned to indicate that a JOIN command failed because the channel requires
 * a key and the key was either incorrect or not supplied
 */
# define ERR_BADCHANNELKEY " 475 "
# define STR_BADCHANNELKEY " :Cannot join channel (+k)"

/**
 * Sent to a client who recently joined a channel with no topic set
 */
# define RPL_NOTOPIC " 331 "
# define STR_NOTOPIC ":No topic is set"

/**
 * Sent to a client who recently joined a channel with the topic, if set
 */
# define RPL_NOTOPIC " 332 "
# define STR_NOTOPIC ":<topic>"

/**
 * The client doesn't have the appropiate channel privileges
 */
# define ERR_CHANOPRIVSNEEDED " 482 "
# define STR_CHANOPRIVSNEEDED ":You're not channel operator"

/**
 * The client isn't part of the channel it's trying to operate
 */
# define ERR_NOTONCHANNEL " 442 "
# define STR_NOTONCHANNEL ":You're not on that channel"

/**
 * Indicates that no client can be found for the supplied nickname
 */
# define ERR_NOSUCHNICK " 401 "
# define STR_NOSUCHNICK ":No such nick/channel"

/**
 * When a client tries to invite <nick> to a channel they’re already joined to
 */
# define ERR_USERONCHANNEL " 443 "
# define STR_USERONCHANNEL ":is already on channel"

/**
 * Sent as a reply to the INVITE command to indicate that the attempt was
 * successful and the client with the user has been invited to the channel.
 */
# define RPL_INVITING " 341 "
# define STR_INVITING ":<channel>>"

/**
 * Indicates that no channel can be found for the supplied channel name
 */
# define ERR_NOSUCHCHANNEL " 403 "
# define STR_NOSUCHCHANNEL ":No such channel"

/**
 * Similar to, but stronger than, ERR_NOSUCHCHANNEL (403)
 */
# define ERR_BADCHANMASK " 476 "
# define STR_BADCHANMASK ":Bad Channel Mask"

/**
 * Returned when a client tries to perform a channel+nick affecting command,
 * when the nick isn’t joined to the channel
 */
# define ERR_USERNOTINCHANNEL " 441 "
# define STR_USERNOTINCHANNEL ":They aren't on that channel"


#endif /* IRC42_NUMERICREPLIES_H */
