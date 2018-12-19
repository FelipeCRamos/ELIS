#ifndef _buffer_h_
#define _buffer_h_

#define MAX_HISTORY_SIZE 10

#include <iostream>
#include <iomanip>
#include <string>
#include <list>
#include <iterator>
#include <ncurses.h>
#include <tuple>
#include <stack>
#include <math.h>
#include "drawer.h"

namespace OP {
    enum RETURN_TYPE{
        NO_ERROR,
        ERROR1,
        ERROR2,
        ERROR3,
        SIG_EXIT,
        NOT_SAVED,
        INVALID_OP
    };

    enum OPERATION{
        SAVE,			// W [<name>]
        EDIT,			// O <name>
        INSERT,			// I [n]
        INSERT_HERE,    // E
        APPEND,			// A [n]
        MOVE_TO_LINE,	// M [n]
        DELETE_LINE,	// D [n [m]]
        HELP,			// H
        QUIT,			// Q
        NULL_OP
    };
};

//!< The '[]' symbol indicates opcional arguments together with command.
std::tuple<OP::OPERATION, std::string, size_t> parse_operation(std::string m_op);


class Buffer{
    private:
        std::list<std::string> screen;          // where all lines will be
        std::list<std::string>::iterator line_number;   // current line

        /* Struct to store a node containing the entire content and its iterator */
        typedef struct history_node_s{
            std::list<std::string> data;
            std::list<std::string>::iterator it;
            history_node_s( std::list<std::string> &curr,
                    std::list<std::string>::iterator c_line )
            {
                this->data = curr;
                this->it = c_line;
            }

            /* Equality operator */
            bool operator ==(const history_node_s &b)
            {
                if( (this->data == b.data) && (this->it == b.it) )
                    return true;
                else return false;
            }

            /* Copy operator */
            std::list<std::string> & operator=( const history_node_s& otr )
            {
                this->data = otr.data;
                this->it= otr.it;
            }
        } snapshot;

        std::stack<snapshot> undo_history;     // to store undo

        size_t min_line = 0;                // default min line
        size_t max_line = TERM_HEIGHT;      // defined on drawer.h
        size_t visible_screen = TERM_HEIGHT - 1;

        bool file_saved = false;            // tells if the changes were made
        bool ignore_warning_saved = false;  // tells if user was warned before

        std::string current_filename;

    public:
        /* Default constructor, initializes with a blank str node */
        Buffer();

        /*
         * Function used for adding new chars, it will append on the end of
         * the current line
         */
        bool add_char(const char *c_);

        /*
         * Function to generate the printable string on the terminal
         * ( summing all lines on the list )
         */
        std::string print(std::string mode_ = "NOT_NAMED");

        /* function that generates and store the snapshot */
        bool save_snapshot();

        /* Set the last snapshot */
        bool prev_snapshot();

        /* Get the current line number (indexed on 0) */
        size_t get_line(void);

        /* Insert a new blank line */
        bool new_line(void);

        /* Function to control the iterator */
        bool walk_to(size_t line_n);

        /* Advance `num` lines */
        bool walk(long int num);

        /* Set line_number to the last element */
        bool walk_to_end(void);

        /* Recalculates the visible screen to be printed (for scroll to work) */
        size_t recalculate_scr(void);

        /* COMMANDS IMPLEMENTATION */
        OP::RETURN_TYPE select_function( OP::OPERATION opr, std::string arg1, size_t arg2 );

        /* SAVE FILE COMMAND */
        OP::RETURN_TYPE save_file(std::string filename);

        /* READ FILE COMMAND */
        OP::RETURN_TYPE read_from_file(std::string filename);

        /* INSERTION ON $line_n LINE */
        OP::RETURN_TYPE insert_on(size_t line_n = 0);

        /* INSERTION ON THE CURRENT LINE */
        OP::RETURN_TYPE insert(void);

        /* APPEND ON $line_n LINE */
        OP::RETURN_TYPE append_on(size_t line_n = 0);

        /* MOVE TO LINE $line_n */
        OP::RETURN_TYPE move_to_line(size_t line_n);

        /* DELETE LINES ON [first, last) */
        OP::RETURN_TYPE delete_line(size_t l_first = 0, size_t l_last = 0);

        /* Prints help text */
        OP::RETURN_TYPE help(void);

        /* QUITS THE PROGRAM */
        OP::RETURN_TYPE quit_p(void);

        bool get_warning(){return this->ignore_warning_saved;};
        bool is_saved(){ return this->file_saved; };

        void set_warning(bool pred){ this->ignore_warning_saved = pred; };
        void set_filename(std::string m_name){this->current_filename = m_name;};
};

#endif
