#include <string>

namespace irc {

/*generic message class */
class Message {

    public:
    Message(void);
    ~Message(void);

    int sendMessage();
    int recvMessage();

    std::string prefix;
    std::string buffer;
    std::string dest;
    std::string src;
};

}