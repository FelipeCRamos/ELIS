#ifndef _drawer_h_
#define _drawer_h_

#include <ncurses.h>    // main drawer
#include <string>       // std::string
#include <iostream>     // std::cout, std::endl...

/* TERM_HEIGHT will recieve the number of lines that the terminal has */
#define TERM_HEIGHT LINES

namespace KEY{
    enum key_type {
        ENTER = 10,             // "enter key"
        BACKSPACE = 8,          // "backspace key"
        DEL = 127,              // "del" key (backspace on some systems)
        COMMAND_INSERT = 58,    // : key
        COMMAND_SEARCH = 47,    // / key
        ESC = 27,               // <Esc> key
        DOWN = 'j',
        UP = 'k',
        UNDO = 'u',
        INSERT_MODE = 'i'
    };
}

class Window{
    public:
        /* Default constructor */
        Window();

        /* Default destructor */
        ~Window();

        /* Main I/O function */
        bool write(std::string m_str);

        void end_session(){
            endwin();
        }
};

#endif
