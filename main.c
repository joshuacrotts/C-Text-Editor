#include <stdlib.h>
#include <termios.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#define CTRL_KEY(k) ((k) & 0x1f)

//
//	Global variables.
//
struct termios orig_termios;

//
//	Function prototypes.
//	
void refresh_screen();

/*
 * Prints an error message to the console and exits with the
 * return code 1.
 *
 */
void die(const char* s) {
	refresh_screen();
	perror(s);
	exit(1);
}

/*
 * Resets the default terminal attributes upon closing the 
 * application.
 */ 
void disable_raw_mode() {
	tcsetattr( STDIN_FILENO, TCSAFLUSH,  &orig_termios );
}

void enable_raw_mode() {
	//	Saves a copy of the current termios attributes
	//	as a reference variable. Upon exit, we restore
	//	them.
	tcgetattr( STDIN_FILENO, &orig_termios );
	atexit( disable_raw_mode );
	
	struct termios raw = orig_termios;
	
	//	Turns off software flow control flags.
	raw.c_iflag &= ~( BRKINT | INPCK | ISTRIP | ICRNL | IXON );
	
	//	Turns off output processing features in the 
	//	default terminal (\r\n).
	raw.c_oflag &= ~( OPOST );
	
	raw.c_cflag |= ( CS8 );
	
	//	ISIG stops CTRL-C/Z/V processes.
	raw.c_lflag &= ~( ECHO | ICANON | ISIG | IEXTEN );
	
	//	Sets a default time out for the console input.
	raw.c_cc[ VMIN ] = 0;			// Sets min # bytes before read() can return.
	raw.c_cc[ VTIME ] = 1;		//	Max amt of time to wait before read() returns.
	
	//	After turning off the ECHO and ICANON flags, we //  set the termios parameters.
	if( tcsetattr( STDIN_FILENO, TCSAFLUSH, &raw ) ) {
		die( "tcsetattr" );
	}
}

/*
 *	Reads a character in from standard IO. If we find an invalid char, we close the
 *  program.
 *
 */
char read_key() {
	int nread;
	char c;
	
	while( ( nread = read( STDIN_FILENO, &c, 1 ) ) != 1 ) {
		if( nread == -1 && errno != EAGAIN ) {
			die( "read" );
		}
	}
	
	return c;
}

/*
 *	For every char input, we process it against certain chars. For instance, if we
 *	are pressing CTRL AND the Q key, we close the program.
 *
 */
void process_key_press() {
	char c = read_key();
	
  switch (c) {
    case CTRL_KEY('q'):
      exit(0);
	  refresh_screen();
      break;
  }
}

/*
 *	Erases all content on the screen and moves the cursor position to the top-left
 *	of the screen.
 */
void refresh_screen() {
	//	Writes two escape sequences to the terminal. Leading chars are \x1b (hex for //	 27), followed by the J command (to clear the screen), that takes in an arg 
	//	(an integer). 2 implies it clears the ENTIRE screen to the end.
	write( STDOUT_FILENO, "\x1b[2J", 4 );
	
	//	Repositions the cursor using the H command using default args of the top-left
	//	corner of the screen.
	write( STDOUT_FILENO, "\x1b[H", 3 );
}

int main() {
	enable_raw_mode();
	
	while( true ) { 
		refresh_screen();
		process_key_press();
	}
	return 0;
}