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


/**
 * The first message sent after client registration
 */
# define RPL_YOURHOST " 002 "
# define STR_YOURHOST " :Your host is <servername>, running version <ver>"


/**
 * The first message sent after client registration
 */
# define RPL_CREATED " 003 "
# define STR_CREATED " :This server was created <date>"


/**
 * The first message sent after client registration
 */
# define RPL_MYINFO " 004 "
# define STR_MYINFO " :<servername> <version> <available user modes><available channel modes>"

/**
 *  Sent as a reply to the AWAY command, this lets the client know that they are no longer set as being away
 */
# define RPL_UMODEIS " 221 "
# define STR_UMODEIS " <user modes>"

/**
 *  Sent to a client to inform that client of their currently-set user modes
 */
# define RPL_UNAWAY " 305 "
# define STR_UNAWAY ":You are no longer marked as being away"

/**
 *  Sent as a reply to the AWAY command, this lets the client know that they are set as being away
 */
# define RPL_NOWAWAY " 306 "
# define STR_NOWAWAY ":You have been marked as being away"

/**
 *  Sent as the final reply of WHOIS command
 */
# define RPL_ENDOFWHOIS " 318 "
# define STR_ENDOFWHOIS " :End of /WHOIS list"

/**
 *  Sent as the first reply to the LIST command
 */
# define RPL_LISTSTART " 321 "
# define STR_LISTSTART " Channel :Users Name"

/**
 *  Sent as the second reply to the LIST command
 */
# define RPL_LIST " 322 "


/**
 *  Sent as the second reply to the MODE command with one parameter
 */
# define RPL_CHANNELMODEIS " 324 "

/**
 *  Sent as the last reply to the LIST command
 */
# define RPL_LISTEND " 323 "
# define STR_LISTEND " :End of /LIST"


/**
 * Sent to a client who recently joined a channel with no topic set
 */
# define RPL_NOTOPIC " 331 "
# define STR_NOTOPIC ":No topic is set"

/**
 * Sent to a client who recently joined a channel with the topic, if set
 */
# define RPL_TOPIC " 332 "

/**
 * Sent as a reply to the INVITE command to indicate that the attempt was
 * successful and the client with the user has been invited to the channel.
 */
# define RPL_INVITING " 341 "

/**
 * Sent as a response to NAMES command
 */
# define RPL_NAMREPLY " 353 "

/**
 * Sent as a response to NAMES command
 */
# define RPL_ENDOFNAMES " 366 "
# define STR_ENDOFNAMES " :End of /NAMES list"

/**
 * Sent as a response to MODE <channel> +b command if blacklist is not empty
 */
# define RPL_BANLIST " 366 "

/**
 * Sent as the final response to MODE <channel> +b command
 */
# define RPL_ENDOFBANLIST " 368 "
# define STR_ENDOFBANLIST " :End of channel ban list"

/**
 * Indicates that no client can be found for the supplied nickname
 */
# define ERR_NOSUCHNICK " 401 "
# define STR_NOSUCHNICK ":No such nick"

/**
 * Indicates that no channel can be found for the supplied channel name
 */
# define ERR_NOSUCHCHANNEL " 403 "
# define STR_NOSUCHCHANNEL ":No such channel"

/**
 * Sent as a response to user trying to send a message to a moderated channel without being moderator
 */
# define ERR_CANNOTSENDTOCHAN " 404 "
# define STR_CANNOTSENDTOCHAN " :You cannot send messages to this channel whilst "

/**
 * JOIN failed because the client has joined their maximum number of channels
 */
# define ERR_TOOMANYCHANNELS " 405 "
# define STR_TOOMANYCHANNELS " :You have joined too many channels"

# define ERR_INPUTTOOLONG " 417 "
# define STR_INPUTTOOLONG " :Input line was too long"

# define ERR_UNKNOWNCOMMAND " 421 "
# define STR_UNKNOWNCOMMAND " :Unknown command"

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
 * Returned when a client tries to perform a channel+nick affecting command,
 * when the nick isn’t joined to the channel
 */
# define ERR_USERNOTINCHANNEL " 441 "
# define STR_USERNOTINCHANNEL ":They aren't on that channel"
/**
 * The client isn't part of the channel it's trying to operate
 */
# define ERR_NOTONCHANNEL " 442 "
# define STR_NOTONCHANNEL ":You're not on that channel"

/**
 * When a client tries to invite <nick> to a channel they’re already joined to
 */
# define ERR_USERONCHANNEL " 443 "
# define STR_USERONCHANNEL ":is already on channel"

/**
 * Not enough parameters were supplied
 */
# define ERR_NOTREGISTERED " 451 "
# define STR_NOTREGISTERED " :You have not registered" 

# define ERR_NEEDMOREPARAMS " 461 "
# define STR_NEEDMOREPARAMS " :Not enough parameters"

/**
 * Client tries to change a detail that can only be set during registration
 */
# define ERR_ALREADYREGISTERED " 462 "
# define STR_ALREADYREGISTERED " :You may not reregister"

# define ERR_PASSWDMISMATCH " 464 "
# define STR_PASSWDMISMATCH " :Password incorrect"

/**
 * Returned to indicate that a MODE argument is unrecognized
 */
# define ERR_UNKNOWNMODE " 472 "
# define STR_UNKNOWNMODE " :is unknown mode char to me"

/**
 * Returned to indicate that a JOIN command failed because the channel is set to
 * invite channelMode and the client has not been invited
 */
# define ERR_INVITEONLYCHAN " 473 "
# define STR_INVITEONLYCHAN " :Cannot join channel (+i)"

/**
 * Returned to indicate that a JOIN command failed because the channel is set to
 * invite channelMode and the client has not been invited
 */
# define ERR_BANNEDFROMCHAN " 474 "
# define STR_BANNEDFROMCHAN " :Cannot join channel (you're banned)"

/**
 * Returned to indicate that a JOIN command failed because the channel requires
 * a key and the key was either incorrect or not supplied
 */
# define ERR_BADCHANNELKEY " 475 "
# define STR_BADCHANNELKEY " :Cannot join channel (+k)"

/**
 * Similar to, but stronger than, ERR_NOSUCHCHANNEL (403)
 */
# define ERR_BADCHANMASK " 476 "
# define STR_BADCHANMASK ":Bad Channel Mask"

/**
 * Returned when user tries to join more than once in a channel
 */
# define ERR_USERONCHANNEL " 443 "
# define STR_USERONCHANNEL ":is already on channel"

/**
 * Returned when a channel doesn't support modes
 */
# define ERR_NOCHANMODES " 477 "
# define STR_NOCHANMODES ":Channel doesn't support modes"

/**
 * The client doesn't have the appropiate channel privileges
 */
# define ERR_CHANOPRIVSNEEDED " 482 "
# define STR_CHANOPRIVSNEEDED ":You're not channel operator"

/**
 * A MODE command affecting a user failed because it's trying to set or view modes for other users.
 */
# define ERR_UMODEUNKNOWNFLAG " 501 "
# define STR_UMODEUNKNOWNFLAG ":Unknown MODE flag"

/**
 * A MODE command affecting a user failed because it's trying to set or view modes for other users.
 */
# define ERR_USERSDONTMATCH " 502 "
# define STR_USERSDONTMATCH ":Cant change mode for other users"

#endif /* IRC42_NUMERICREPLIES_H */
