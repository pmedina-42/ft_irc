#ifndef IRC42_NUMERICREPLIES_H
# define IRC42_NUMERICREPLIES_H

/* RFC 2812 section 2.4 (page 7) :
 * The numeric reply MUST be sent as one message consisting of
 * the sender prefix, the three-digit numeric, and the target of
 * the reply.
 */

/**
 * NICK & USER numeric replies
 */
# define RPL_WELCOME " 001 "
# define RPL_WELCOME_STR_1 " :Welcome to the Internet Relay Network, "

# define ERR_NONICKNAMEGIVEN " 431 "
# define STR_NONICKNAMEGIVEN " :No nickname given"

# define ERR_ERRONEUSNICKNAME " 432 "
# define STR_ERRONEUSNICKNAME " :Erroneus nickname"

# define ERR_NICKNAMEINUSE " 433 "
# define STR_NICKNAMEINUSE " :Nickname is already in use"

# define ERR_NEEDMOREPARAMS " 461 "
# define STR_NEEDMOREPARAMS " :Not enough parameters"

# define ERR_ALREADYREGISTERED " 462 "
# define STR_ALREADYREGISTERED " :You may not reregister"

/**
 * JOIN numeric replies
 */
# define ERR_TOOMANYCHANNELS " 405 "
# define STR_TOOMANYCHANNELS " :You have joined too many channels"

# define ERR_BADCHANNELKEY " 475 "
# define STR_BADCHANNELKEY " :Cannot join channel (+k)"

#endif /* IRC42_NUMERICREPLIES_H */
