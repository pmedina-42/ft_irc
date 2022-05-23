#ifndef IRC42_EXCEPTIONS_H
#define IRC42_EXCEPTIONS_H

#include <exception>

namespace irc {
namespace exc {

class MallocError : public std::exception {
    public:
    virtual const char* what() const throw() {
        return( "Fatal error (malloc)" );
    }
};

class ServerSetUpError : public std::exception {
    public:
    virtual const char* what() const throw() {
        return ("Server : Set up error");
    }
};

}
}

#endif /* IRC42_EXCEPTIONS_H */