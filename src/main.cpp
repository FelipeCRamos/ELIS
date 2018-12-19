#include <iostream>
#include "drawer.h"
#include "buffer.h"

#define M_INS       "INSERT"
#define M_NORMAL    "NORMAL"
#define M_COMMAND   "COMMAND"

bool get_key( char *buf ) {
    try {
        *buf = getch();
        return true;
    } catch (...) {
        return false;
    }
}

enum MODE {
    INSERT,     // stands for the text insertion mode
    NORMAL,     // stands for normal mode
    COMMAND     // stands for command insertion mode
};


int main(int argc, char ** argv){
    // Initializes the ncurses window
    Window myTerm;

    /* Initializes the Buffer, made for manage the strings doubly-linked list */
    Buffer myBuf;

    /* Initial snapshot */
    myBuf.save_snapshot();

    /* Welcome screen message */
    myTerm.write("~ Welcome to the ELIS Project!");

    /* ARGS TREAT */
    if(argc == 2){
        /* file name from command line */
        std::string file_name(argv[1]);
        /* reads content from the file */

        myBuf.set_warning(true);
        OP::RETURN_TYPE function_return = 
            myBuf.read_from_file(std::string(argv[1]));

        /* ERROR HANDLING */
        if(function_return == OP::ERROR1)
        {
            myTerm.write("~ Corrupted File! ELIS wasn't able to open it!\
                    \n\nPress any key to exit!");
            getch();        // wait for the key press
            return 1;       // exit the program
        }

        /* print initial state of the program */
        myTerm.write(myBuf.print(M_NORMAL));

    } else if(argc > 2) {
        myTerm.end_session();   // end session to print the error msg
        std::cerr << "elis: ";
        std::cerr << "Invalid number of arguments. More info on documentation\n";
        return 1;
    }


    /* Set initial mode */
    MODE current_mode = NORMAL;

    char *curr_key = new char;
    std::string curr_command;

    bool st_prev;                               // debug variable
    bool came_from_insert_function = false;     // another debug variable

    OP::RETURN_TYPE function_return;            // variable to hold errors

    /* Main program loop, it will wait for keys to be pressed */
    while( get_key(curr_key) ){
        if( current_mode == NORMAL ){
            /* NORMAL MODE */
            std::string print_name = M_NORMAL;

            switch(*curr_key){
                case KEY::ENTER:
                    /* Simply walks one line below */
                    myBuf.walk(1);
                    break;

                case KEY::COMMAND_INSERT:
                    /*
                     * : KEY PRESSED!
                     * change the current_mode for command insertion.
                     */
                    current_mode = COMMAND;
                    print_name = M_COMMAND;
                    break;

                case KEY::UP:
                    /* Change iterator on myBuf to one position above */
                    myBuf.walk(-1);
                    break;

                case KEY::DOWN:
                    /* Change iterator on myBuf to one position below */
                    myBuf.walk(1);
                    break;

                case KEY::UNDO:
                    /* go back to previous snapshot */
                    st_prev = myBuf.prev_snapshot();
                    break;

                case KEY::INSERT_MODE:
                    current_mode = INSERT;
                    print_name = M_INS;
                    break;
            }

            /* then, redraws the terminal window */
            myTerm.write(myBuf.print(print_name));

        } else if ( current_mode == INSERT ) {
            /* INSERTION MODE */
            std::string print_name = M_INS;

            switch(*curr_key) {
                case KEY::ESC:
                    myBuf.save_snapshot();  // save snapshot
                    came_from_insert_function = false;

                    /* change current_mode back to 'NORMAL' */
                    current_mode = NORMAL;
                    print_name = M_NORMAL;
                    break;

                default:
                    /* add's the char to the Buffer */
                    myBuf.add_char(curr_key);
                    break;
            }

            /* then, redraws the terminal window */
            myTerm.write(myBuf.print(print_name));

        } else if( current_mode == COMMAND ) {
            /* COMMAND MODE */
            std::string print_name = M_COMMAND;

            /* warning message, will be printed later if functions
             * return != OP::NO_ERROR */
            std::string warning;

            /* variable to hold the operation details */
            std::tuple<OP::OPERATION, std::string, size_t> t_cmd;

            /* control boolean */
            bool function_executed = false;

            switch(*curr_key) {
                case KEY::ESC:
                    /* Delete buffer command, go back to normal mode */
                    curr_command.clear();
                    current_mode = NORMAL;
                    print_name = M_NORMAL;
                    break;

                case KEY::ENTER:
                    /* Execute command on the buffer till now */
                    t_cmd = parse_operation(curr_command);

                    /* ^
                     *  std::get<0>(t_cmd) = opcode
                     *  std::get<1>(t_cmd) = std::string (arg1)
                     *  std::get<2>(t_cmd) = size_t (arg2)
                     */

                    curr_command.clear();   // resets variable for the next cmd

                    /* variable to hold result from functions */
                    // OP::RETURN_TYPE function_return;

                    /* FUNCTION CALLS */
                    function_return = myBuf.select_function( std::get<0>(t_cmd),
                            std::get<1>(t_cmd),
                            std::get<2>(t_cmd)
                        );

                    /* control variable, to check if has to print the `:` */
                    function_executed = true;

                    /* ERROR HANDLING */
                    switch(function_return)
                    {
                        case OP::SIG_EXIT:
                            delete curr_key;
                            return 0;       // exit program

                        case OP::NO_ERROR:
                            break;  // TODO

                        case OP::ERROR1:
                            /* if returns == OP::ERROR1, let's treat the error
                             * by depending on his function */
                            switch( std::get<0>(t_cmd) )
                            {
                                case OP::SAVE:      // ERROR1 with save function
                                    warning = "ERROR: No filename provided!";
                                    break;
                            }
                            break;

                        case OP::ERROR2:
                            /* if returns == OP::ERROR2, let's treat the error
                             * by depending on his function */
                            switch( std::get<0>(t_cmd) )
                            {
                                case OP::SAVE:      // ERROR2 with save function
                                    warning = "ERROR: Could not write on file!";
                                    break;
                            }
                            break;

                        case OP::NOT_SAVED:
                            /* if returns == OP::NOT_SAVED, let's treat the error
                             * by depending on his function */
                            switch( std::get<0>(t_cmd) ){
                                case OP::EDIT:
                                    /* i know this line is too long */
                                    warning = "WARNING: Progress not saved, run `E` again to ignore it.";

                                    /* now the user has been warned */
                                    myBuf.set_warning(true);
                                    break;
                            }

                        case OP::ERROR3:
                            break;  // TODO

                        case OP::INVALID_OP:
                            break;  // TODO

                        default:
                            break;
                    }

					/* These two need to go back to insertion mode */
					if( std::get<0>(t_cmd) == OP::INSERT or
                        std::get<0>(t_cmd) == OP::APPEND or
                        std::get<0>(t_cmd) == OP::INSERT_HERE)
                    {
                        came_from_insert_function = true;   // control variable

                        /* set the mode to INSERT */
						current_mode = INSERT;
						print_name = M_INS;

					} else {
                    	/* Go back to normal mode */
                    	current_mode = NORMAL;
                    	print_name = M_NORMAL;
					}
					break;

                case KEY::DEL:
                case KEY::BACKSPACE:
                    if( !curr_command.empty() )
                        curr_command.pop_back();    // delete char on curr_command
                    break;

                default:
                    /* Starts to capture the command */
                    curr_command.append(curr_key);
                    break;
            }

            /* Redraws terminal window */
            if( !function_executed )
                myTerm.write(myBuf.print(print_name) + ":" + curr_command);
            else if ( function_return == OP::NO_ERROR )
                myTerm.write(myBuf.print(print_name));
            else
                myTerm.write(myBuf.print(print_name) + warning);
        }
    }

    delete curr_key;
    return 0;
}
