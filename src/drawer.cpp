#include "drawer.h"

Window::Window(){
    initscr();      // Initializes screen
    noecho();       // Disables eco on screen
    refresh();      // Refreshes screen to match memory content
}

Window::~Window(){
    endwin();       // Detaches ncurses session and clear memory
}

bool Window::write(std::string m_str){
    try {
        clear();        // clear the actual screen

        /* Write on the screen all the content from m_str */
        for( int i = 0; i < m_str.size(); i++)
            printw("%c", m_str[i]);

        refresh();      // refreshes the screen

        return true;    // write successfully

    } catch (...) {
        std::cerr << "Error on write() function!\n";
        return false;       // could not write on terminal for some error (?)
    }
}
