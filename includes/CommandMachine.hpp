#ifndef IRC42_COMMAND_MACHINE_H
# define IRC42_COMMAND_MACHINE_H

struct Command;

class CommandMachine {
    
    public:
    CommandMachine();
    ~CommandMachine();



    private:
    std::list<Command>;

};

#endif