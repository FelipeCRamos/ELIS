#include "buffer.h"
#include <tuple>
#include <fstream>

/* Default constructor */
Buffer::Buffer()
{
    this->screen.push_back("");
    this->line_number = this->screen.begin();
}

std::string format_number(int max_len, int number){
    std::string buf;
    int spaces_count = max_len - floor(log10(number));
    for( int i = 0; i < spaces_count; i++ ){
        buf.append(" ");
    }
    buf.append(std::to_string(number));
    return buf;
}

bool Buffer::add_char(const char *c_)
{
    std::list<std::string>::iterator it = line_number;

    if( *c_ == KEY::ENTER ){
        /* if enter, creates a new node on list and change to it */
        this->screen.insert(++it, std::string(""));
        this->line_number++;
        return true;
    } else if( *c_ == KEY::DEL || *c_ == KEY::BACKSPACE ) {
        /* delete last char on str */
        if(!this->line_number->empty())
            this->line_number->pop_back();
        return true;
    } else {
        try{
            /* add *c_ to the end of str */
            it->push_back(*c_);
            return true;
        } catch (...) {
            /* not thinked */
            return false;
        }
    }
}

std::string Buffer::print(std::string mode_)
{
    /* local buffer, just necessary to make the std::string */
    std::string buf;

    /* Does the scroll calculation */
    size_t curr = recalculate_scr();

    /* line counter */
    int line = 0;
    size_t printed_lines = 0;
    int max_num_digits = log10(this->screen.size());

    for(auto str_ = screen.begin(); str_ != screen.end(); str_++, line++){
        if( line < max_line && line >= min_line ){
            /* line number */
            // buf.append(std::to_string(line + 1));   // index começando em 1!!!
            buf.append( format_number(max_num_digits, line+1) );

            /* indicator of the current line */
            (this->line_number == str_) ? buf.append("*> ") : buf.append(" > ");

            /* string to be appended */
            buf.append(*str_);

            printed_lines++;

            /* newline char */
            buf.append("\n");
        }
    }
    for( int i = 0; i < visible_screen - printed_lines; i++ ){
        buf.append("~\n");
    }
    /* Statusbar print */
    buf.append(" ~ ");
    buf.append(mode_);
    buf.append(" ~ ");
    // buf.append(std::to_string(rdm));

    /* returning the final string */
    return buf;
}

bool Buffer::save_snapshot()
{
    /* Creates a snapshot */
     snapshot sn = snapshot(this->screen, this->line_number);

    /* Add's to the snapshot list */
    if( this->undo_history.empty() ) {
        this->undo_history.push(sn);
    } else {
        if( !(this->undo_history.top() == sn)){
            this->undo_history.push(sn);
            this->file_saved = false;   // the user made a change, needs to save
        }
    }

    return true;
}

bool Buffer::prev_snapshot()
{
    /* removes the actual state */
    if( !this->undo_history.empty() && this->screen == this->undo_history.top().data ){
        this->undo_history.pop();
    }

    /* then set the last state (if has any) */
    if( !this->undo_history.empty() )
    {
        snapshot sn = this->undo_history.top();
        this->screen = sn.data;
        this->line_number = this->screen.begin();   // this works
        // this->line_number = sn.it;               // this don't
        return true;
    }

    return false;
}

size_t Buffer::get_line()
{
    return std::distance(this->screen.begin(), this->line_number);
}

bool Buffer::new_line(void)
{
    char c = '\n';
    this->add_char(&c);

    return true;
}

bool Buffer::walk_to(size_t line_n)
{
    this->line_number = this->screen.begin();   // set to begin
    for( int i = 0; i < line_n; i++ ){
        try{
            this->walk(1);
        } catch (...) {
            return false;
        }
    }
    return true;
}

bool Buffer::walk(long int num)
{
    /* empty buffer, do nothing */
    if( this->screen.empty() ) return false;

    if( this->line_number == this->screen.begin() && num < 0 ) {
        return false;
    }
    try {
        std::advance(this->line_number, num);

        if( this->line_number == this->screen.end() ){
            std::advance(this->line_number, -1);
            return false;
        }

        return true;
    } catch (...) {
        return false;
    }
}

bool Buffer::walk_to_end(void)
{
    if( this->screen.empty() ) return false;    // if empty, nothing to do

    auto it = std::prev(this->screen.end(), 1); // set it to the last element
    this->line_number = it;
    return true;
}

size_t Buffer::recalculate_scr(void)
{
    size_t curr_cursor_line = std::distance( screen.begin(), line_number );
    if( curr_cursor_line >= max_line ) {
        max_line = curr_cursor_line + 1;
        min_line = max_line - visible_screen;
    } else if( curr_cursor_line <= min_line ) {
        min_line = curr_cursor_line;
        max_line = min_line + visible_screen;
    }
    return curr_cursor_line;
}

/* COMMANDS IMPLEMENTATION */

std::tuple<OP::OPERATION, std::string, size_t> parse_operation(std::string m_op)
{
    std::list<std::string> words;
    std::string curr_w;
    for( auto it = m_op.begin(); it != m_op.end(); it++ )
    {
        if(*it != ' ')
        {
            curr_w += *it;
        } else {
            if( curr_w.empty() ){
				return std::make_tuple(OP::NULL_OP, "0", 0);
			}
            words.push_back(curr_w);
            curr_w.clear();
        }
    }
    if(!curr_w.empty()) words.push_back(curr_w);

    curr_w.clear();

    // security features (prevent segfaults)
    if( words.size() > 3 ) return std::make_tuple(OP::NULL_OP, "0", 0);
    if( words.empty() ) return std::make_tuple(OP::NULL_OP, "0", 0);

    /* At this point, words should be populated, let's tokenize */
    OP::OPERATION selected_op;

    if( words.front() == "W" || words.front() == "w" ) selected_op = OP::SAVE;
    else if( words.front() == "E" || words.front() == "e" ) selected_op = OP::INSERT_HERE;
    else if( words.front() == "O" || words.front() == "o" ) selected_op = OP::EDIT;
    else if( words.front() == "I" || words.front() == "i" ) selected_op = OP::INSERT;
    else if( words.front() == "A" || words.front() == "a" ) selected_op = OP::APPEND;
    else if( words.front() == "M" || words.front() == "m" ) selected_op = OP::MOVE_TO_LINE;
    else if( words.front() == "D" || words.front() == "d" ) selected_op = OP::DELETE_LINE;
    else if( words.front() == "H" || words.front() == "h" ) selected_op = OP::HELP;
    else if( words.front() == "Q" || words.front() == "q" ) selected_op = OP::QUIT;
    else selected_op = OP::NULL_OP;

    words.pop_front();
    // now words = [ arg1, arg2 ], not [ op, arg1, arg2 ]

    std::string arg = "";
    switch(selected_op)
    {
        case OP::SAVE:
        case OP::EDIT:
            try{
                arg = *(words.begin());
            }catch(...){}
            return std::make_tuple(selected_op, arg, 0);

        case OP::INSERT:
        case OP::APPEND:
        case OP::MOVE_TO_LINE:
			/*if( words.front() == "" ) words.front() = "0"; */
            if( words.size() == 0 ) return std::make_tuple(selected_op, "0", 0);
            return std::make_tuple(selected_op, *(words.begin()), 0);
            break;

        case OP::DELETE_LINE:
			if( words.size() == 0) return std::make_tuple(selected_op, "0", 0);
			else if(words.size() == 1){
				return std::make_tuple(selected_op, words.front(), 0);
			} else {
				return std::make_tuple( selected_op,            // op
										words.front(),          // arg1
										std::stoi(words.back()) // arg2
									  );
			}
            break;

        case OP::HELP:
        case OP::QUIT:
            return std::make_tuple(selected_op, "0", 0);

        case OP::INSERT_HERE:
            return std::make_tuple(selected_op, "0", 0);
            break;

        default:
            return std::make_tuple(OP::NULL_OP, "0", 0);
    }
}

OP::RETURN_TYPE Buffer::select_function( OP::OPERATION opr, std::string arg1, size_t arg2 )
{

    size_t c_arg1;   // if needs convertion
    OP::RETURN_TYPE curr_ret;

    if( opr == OP::SAVE || opr == OP::EDIT )
    {
        /* The only ones that needs the arg1 to be a std::string */
        switch(opr)
        {
            case OP::SAVE:
                return save_file(arg1);

            case OP::EDIT:
                /* generate the snapshot */
                save_snapshot();
                return read_from_file(arg1);

            default:
                return OP::INVALID_OP;
        }
    } else {
        /* The rest, that needs the arg1 to be a size_t */
        try{
            c_arg1 = stoi(arg1);
        } catch (...) {
            return OP::INVALID_OP;
        }
        switch(opr)
        {
            case OP::INSERT:
                curr_ret = insert_on(c_arg1);
                return curr_ret;

            case OP::INSERT_HERE:
                curr_ret = insert();
                return curr_ret;

            case OP::APPEND:
                curr_ret = append_on(c_arg1);
                return curr_ret;

            case OP::MOVE_TO_LINE:
                return move_to_line(c_arg1);

            case OP::DELETE_LINE:
                /* then, generate the snapshot */
                save_snapshot();
				if(c_arg1 > arg2) std::swap(c_arg1, arg2);
                return delete_line(c_arg1, arg2);

            case OP::HELP:
                return help();

            case OP::QUIT:
                return quit_p();

            default:
                return OP::INVALID_OP;
        }
    }
}

OP::RETURN_TYPE Buffer::save_file(std::string filename){
    if(filename == ""){
        if(this->current_filename == ""){
            /* if no file name was passed and there is no current file name */
            return OP::ERROR1;
        }else{
            filename = this->current_filename;
        }
    }else if(this->current_filename == ""){
        this->current_filename = filename;
    }

    /* opens stream and check for erros*/
    std::ofstream ofs;
    ofs.exceptions ( std::ofstream::failbit );
    try {
        ofs.open(filename.c_str());
    }
    catch (std::ofstream::failure e) {
        return OP::ERROR2;
    }

    /* passes each string on the list to the output file stream */
    for(auto str_ = screen.begin(); str_ != screen.end(); str_++){
        ofs << *str_ << std::endl;
    }

    this->file_saved = true;
    ofs.close();

    return OP::NO_ERROR;
}

/* READ FILE COMMAND */
OP::RETURN_TYPE Buffer::read_from_file(std::string filename){

    /* if the current state wasn't saved AND the user wasn't warned yet */
    if( !this->file_saved && !ignore_warning_saved )
        return OP::NOT_SAVED;

    this->ignore_warning_saved = false;
    /* the current file will be the file that is being loaded */
    this->current_filename = filename;

    /* a temporary list of strings to save retrieve the file to the program */
    std::list<std::string> temp_screen;
    temp_screen.push_back("");
    /* opens stream */
    std::ifstream ifs;
    ifs.exceptions ( std::ifstream::failbit );
    try{
        ifs.open(filename);
    } catch (std::ifstream::failure e) {
        this->screen.clear();
        this->screen.push_back("");
        this->line_number = this->screen.begin();
        return OP::NO_ERROR;
    }

    /* if the read char is a \n, we add a new line(string) into our list
       otherwise, we add the char to the current line */
    char c;
    // ifs.exceptions ( std::ifstream:: );
    while(ifs.good()){
        c = ifs.get();
        if(c == EOF){
            break;
        }else if(c == '\n' && ifs.peek() != EOF){
            temp_screen.push_back("");
        } else {
            temp_screen.back().push_back(c);
        }
    }

    /* if the reading was aborted due to a badbit, sends error */
    if(ifs.bad()){
        return OP::ERROR1;
    }

    /* if everything went well, we replace the current screen with the temp */

    if( !temp_screen.back().empty() )
        temp_screen.back().pop_back();      // eliminates final \n
    this->screen = temp_screen;
    this->line_number = this->screen.begin();

    /* clearing the temp object and closing the stream */
    temp_screen.clear();
    ifs.close();

    return OP::NO_ERROR;
}

OP::RETURN_TYPE Buffer::insert_on(size_t line_n)
{
    bool walk_b;

    // if arg == 0, then we should insert on the above line
    if( line_n == 0 ) {
        walk_b = walk(-1);

    // if not, we should insert above the `line_n-1` line
    } else if( screen.size() > line_n ) {
        walk_to(line_n - 1);
        walk_b = walk(-1);

    // if the line doesn't exists, we should insert on the end-1 line
    } else {
        walk_to_end();
        walk_b = walk(-1);
    }

    // then, if the line was there, we just create a new line, else, we push
    if( !walk_b ) {
        screen.push_front(std::string(""));
        walk(-1);

    } else {
        new_line();
    }
    return OP::NO_ERROR;
}

OP::RETURN_TYPE Buffer::insert(void)
{ 
    /* Insert on the current line */
    return OP::NO_ERROR;    // placeholder
}

OP::RETURN_TYPE Buffer::append_on(size_t line_n)
{
    /* Check if the line stays into the range [1, this->screen.size()] */
    if(line_n >= this->screen.size()) {
        this->walk_to_end();
    }

	/* First of all, needs checking either argument was given or not. */
    else if(line_n != 0){
		/* User-given argument is invalid. */
		try {
			this->walk_to(line_n-1);
		} catch (...) {
			return OP::ERROR1;
		}
	}

    /* Creates a new line after this->line_number */
    this->new_line();

	// With these I reach the right line. Now, insertion is required.
	return OP::NO_ERROR;
}

OP::RETURN_TYPE Buffer::move_to_line(size_t line_n)
{
    /* if the line is bigger than the lines on the document */
    if( line_n == 0 || line_n >= this->screen.size() ) {
        walk_to_end();
        return OP::NO_ERROR;
    }

    else if( walk_to(line_n-1) ) {
        return OP::NO_ERROR;
    }

    else return OP::ERROR1;
}

OP::RETURN_TYPE Buffer::delete_line(size_t l_first, size_t l_last)
{
	/* First checks if arguments were given */
	if( l_first != 0)
	{
		std::list<std::string>::iterator it = this->screen.begin();
		//if( l_first < 0 ) l_first = 1;
		if( l_first >= this->screen.size() ) l_first = this->screen.size();
		std::advance(it, l_first);

		if( l_last != 0)
		{
			std::list<std::string>::iterator itr = this->screen.begin();
			/* Adjusts if given number is bigger than screen's size. */
			if( l_last >= this->screen.size() ) l_last = this->screen.size();
			std::advance(itr, l_last);

			size_t first = std::distance(this->screen.begin(), --it);
			size_t last = std::distance(this->screen.begin(), itr);
			size_t pos = std::distance(this->screen.begin(), this->line_number);
			/* Checks if current line will be deleted */
			/* If so, some adjustments onto current line number. */
			if( first <= pos && pos <= last ){
				this->line_number = itr;
			}
			this->screen.erase(it, itr);
		}
		else{
			/* Checks if current line is the one being deleted. */
			if( --it == line_number ) this->line_number++;
			this->screen.erase(it);
		}
	} else {
		auto temp = line_number;
		temp++;
		this->screen.erase(line_number);
		this->line_number = temp;
	}
	/* Adjustment to situations where last line is being deleted
		and current line is there. */
	if( this->line_number == this->screen.begin() ) this->new_line();
	else if( this->line_number == this->screen.end() ) this->line_number--;
	return OP::NO_ERROR;
}

OP::RETURN_TYPE Buffer::help(void)
{
    system("less include/help.man");  // quick workaround :P
    return OP::NO_ERROR;
}

OP::RETURN_TYPE Buffer::quit_p(void)
{
    // implement the check if user has saved
    return OP::SIG_EXIT;
}
