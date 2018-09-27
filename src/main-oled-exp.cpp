#include <oled-exp.h>
#include <onion-debug.h>

// function prototypes:
void 	Usage 				(const char* progName);
int 	command 		(char *command, char *param);
fastDebuger dbg;
fastOledDriver drv;

// print the usage info 
void usage(const char* progName) 
{
	dbg.print(ONION_SEVERITY_FATAL, "\n");
	dbg.print(ONION_SEVERITY_FATAL, "Usage: oled-exp -i\n");
	dbg.print(ONION_SEVERITY_FATAL, "\n");
	dbg.print(ONION_SEVERITY_FATAL, "FUNCTIONALITY:\n");
	dbg.print(ONION_SEVERITY_FATAL, "  Initialize the OLED Display\n");
	dbg.print(ONION_SEVERITY_FATAL, "\n\n");

	dbg.print(ONION_SEVERITY_FATAL, "Usage: oled-exp -c\n");
	dbg.print(ONION_SEVERITY_FATAL, "\n");
	dbg.print(ONION_SEVERITY_FATAL, "FUNCTIONALITY:\n");
	dbg.print(ONION_SEVERITY_FATAL, "  Clear the OLED Display\n");
	dbg.print(ONION_SEVERITY_FATAL, "\n\n");

	dbg.print(ONION_SEVERITY_FATAL, "Usage: oled-exp [-icqv] COMMAND PARAMETER\n");
	dbg.print(ONION_SEVERITY_FATAL, "\n");
	dbg.print(ONION_SEVERITY_FATAL, "The following COMMANDs are available:\n");
	dbg.print(ONION_SEVERITY_FATAL, "  power <on|off>                  Turn the display on or off\n");
	dbg.print(ONION_SEVERITY_FATAL, "  write <message>                 Write the input string on the display\n");
	dbg.print(ONION_SEVERITY_FATAL, "  writeByte <byte>                Write the input byte on the display\n");
	dbg.print(ONION_SEVERITY_FATAL, "  dim <on|off>                    Adjust the screen brightness\n");
	dbg.print(ONION_SEVERITY_FATAL, "  invert <on|off>                 Invert the colors on the display\n");
	dbg.print(ONION_SEVERITY_FATAL, "  cursor <row>,<column>           Set the cursor to the specified row and column\n");
	dbg.print(ONION_SEVERITY_FATAL, "  cursorPixel <row>,<pixel>       Set the cursor to the specified row and pixel\n");
	dbg.print(ONION_SEVERITY_FATAL, "  scroll <direction>              Enable scrolling of screen content\n");
	dbg.print(ONION_SEVERITY_FATAL, "         available directions:    left, right, diagonal-left, diagonal-right\n");
	dbg.print(ONION_SEVERITY_FATAL, "         to stop scrolling:       stop\n");
	dbg.print(ONION_SEVERITY_FATAL, "  draw <lcd file>                 Draw the contents of an lcd file to the display\n");
	dbg.print(ONION_SEVERITY_FATAL, "\n");
	dbg.print(ONION_SEVERITY_FATAL, "COMMANDs can be cascaded one after another, they will execute in order.\n");
	dbg.print(ONION_SEVERITY_FATAL, "\n");

	dbg.print(ONION_SEVERITY_FATAL, "OPTIONS:\n");
	dbg.print(ONION_SEVERITY_FATAL, "  -i 		initialize display\n");
	dbg.print(ONION_SEVERITY_FATAL, "  -c 		clear the display\n");
	dbg.print(ONION_SEVERITY_FATAL, "  -q 		quiet: no output\n");
	dbg.print(ONION_SEVERITY_FATAL, "  -v 		verbose: lots of output\n");
	dbg.print(ONION_SEVERITY_FATAL, "  -h 		help: show this prompt\n");
	

	dbg.print(ONION_SEVERITY_FATAL, "\n");
}

// execute a specified command
int command(char *command, char *param)
{
	int 	val0, val1;
	uint8_t	*buffer;

	// perform the specified command
	dbg.print(ONION_SEVERITY_DEBUG_EXTRA, "command = '%s', param = '%s'\n", command, param);
	if (strcmp(command, "write") == 0 ) {	
		drv.write(param);
	}
	else if (strcmp(command, "writeByte") == 0 ) {	
		// parse the byte
		if (param[0] == '0' && param[1] == 'x') {
			sscanf(param, "0x%02x", &val0);
		}
		else {
			sscanf(param, "%02x", &val0);
		}
		drv.writeByte(val0);
	}
	else if (strcmp(command, "brightness") == 0 ) {
		drv.setBrightness( atoi(param) );
	}
	else if (strcmp(command, "invert") == 0 ) {
		// interpret the parameter
		val0 	= 0;	// off by default
		if (strcmp(param, "on") == 0 ) {
			val0 = 1;
		}
		drv.setDisplayMode( val0 );
	}
	else if (strcmp(command, "power") == 0 ) {
		// interpret the parameter
		val0 	= 0;	// off by default
		if (strcmp(param, "on") == 0 ) {
			val0 = 1;
		}
		drv.setDisplayPower(val0);
	}
	else if (strcmp(command, "dim") == 0 ) {
		// interpret the parameter
		val0 	= 0;	// off by default
		if (strcmp(param, "on") == 0 ) {
			val0 = 1;
		}
		drv.setDim(val0);
	}
	else if (strcmp(command, "cursor") == 0 ) {
		// interpret the parameter
		sscanf(param, "%d, %d", &val0, &val1);
		dbg.print(ONION_SEVERITY_INFO, "> Setting cursor to (%d, %d)\n", val0, val1);
		drv.setTextColumns();
		drv.setCursor(val0, val1);
	}
	else if (strcmp(command, "cursorPixel") == 0 ) {
		// interpret the parameter
		sscanf(param, "%d, %d", &val0, &val1);
		dbg.print(ONION_SEVERITY_INFO, "> Setting cursor to row: %d, pixel: %d\n", val0, val1);
		drv.setImageColumns();
		drv.setCursorByPixel(val0, val1);
	}
	else if (strcmp(command, "draw") == 0 ) {
		// allocate memory for the buffer
		buffer 	= malloc(OLED_EXP_WIDTH*OLED_EXP_HEIGHT/8 * sizeof *buffer);
		
		memset(buffer, 0, OLED_EXP_WIDTH*OLED_EXP_HEIGHT/8 * sizeof *buffer); // FIXME: We should definitely do a #define for the buffer size calculation. This looks ugly.

		// read the parameter
		if ( strncmp(param, OLED_EXP_READ_LCD_DATA_IDENTIFIER, strlen(OLED_EXP_READ_LCD_DATA_IDENTIFIER) ) == 0 ) {
			dbg.print(ONION_SEVERITY_INFO, "> Reading data from argument\n");

			dbg.print(ONION_SEVERITY_DEBUG_EXTRA, "  param length is %d\n", strlen(param) );
			// remove the data identifier from the string
			memmove	(	param, 
						param + strlen(OLED_EXP_READ_LCD_DATA_IDENTIFIER), 
						strlen(param) 
					);
			dbg.print(ONION_SEVERITY_DEBUG_EXTRA, "  after move: param length is %d\n", strlen(param) );

			// read the data into a buffer
			drv.readLcdData(param, buffer);
		}
		else {
			// read data from a file
			dbg.print(ONION_SEVERITY_INFO, "> Reading data from file '%s'\n", param);
			drv.readLcdFile(param, buffer);
		}

		drv.draw(buffer, OLED_EXP_WIDTH*OLED_EXP_HEIGHT/8);
		/*}
		else {
			dbg.print(ONION_SEVERITY_FATAL, "ERROR: Cannot draw invalid data\n");
		}*/

		// deallocate memory for the buffer
		if (buffer != NULL) {
			dbg.print(ONION_SEVERITY_DEBUG_EXTRA, "> Deallocating buffer array\n");
			free(buffer);
		}
	}
	else if (strcmp(command, "scroll") == 0 ) {
		// interpret the parameters
		val0 		= -1;
		val1 		= -1;
		if (strcmp(param, "left") == 0) {
			val0 	= 0;	// horizontal scrolling
			val1	= 0;	// scrolling left
		}
		else if (strcmp(param, "right") == 0) {
			val0 	= 0;	// horizontal scrolling
			val1	= 1;	// scrolling right
		}
		else if (strcmp(param, "diagonal-left") == 0) {
			val0 	= 1;	// vertical scrolling
			val1	= 0;	// scrolling up
		}
		else if (strcmp(param, "diagonal-right") == 0) {
			val0 	= 1;	// vertical scrolling
			val1	= 1;	// scrolling down
		}

		if (val0 == -1) {
			drv.scrollStop();
		}
		else if (val0 == 0) {
			// horizontal scrolling
			drv.scroll(val1, OLED_EXP_SCROLL_SPEED_5_FRAMES, 0, OLED_EXP_CHAR_ROWS-1);
		}
		else if (val0 == 1) {
			// diagonal scrolling
			drv.scrollDiagonal (	val1, 								// direction
											OLED_EXP_SCROLL_SPEED_5_FRAMES, 	// scroll speed
											0, 									// # fixed rows
											OLED_EXP_HEIGHT, 					// # scrolling rows
											1, 									// rows to scroll by
											0, 									// horizontal start page
											OLED_EXP_CHAR_ROWS-1				// horizontal end page	
										);
		}
	}
	else {
		dbg.print(ONION_SEVERITY_FATAL, "> Unrecognized command '%s'\n", command );
	}

	return EXIT_SUCCESS;
}

int main(int argc, char** argv)
{
	dbg = fastDebuger(0);
	drv = fastOledDriver();
	const char *progname;
	char 	*command;
	char 	*param;
	
	int 	status;
	int 	verbose;
	int 	init;
	int 	clear;
	int 	ch;


	// set the defaults
	init 		= 0;
	clear 		= 0;
	verbose 	= ONION_VERBOSITY_NORMAL;

	command 	= malloc(MAX_COMMAND_LENGTH * sizeof *command);
	param 		= malloc(MAX_PARAM_LENGTH * sizeof *param);

	// save the program name
	progname 	= argv[0];	


	//// parse the option arguments
	while ((ch = getopt(argc, argv, "vqhic")) != -1) {
		switch (ch) {
		case 'v':
			// verbose output
			verbose++;
			break;
		case 'q':
			// quiet output
			verbose = ONION_VERBOSITY_NONE;
			break;
		case 'i':
			// perform init sequence
			init 	= 1;
			break;
		case 'c':
			// perform clear sequence
			clear 	= 1;
			break;
		default:
			usage(progname);
			return 0;
		}
	}

	// set the verbosity
	onionSetVerbosity(verbose);


	// advance past the option arguments
	argc 	-= optind;
	argv	+= optind;


	//// OLED PROGRAMMING
	// check if OLED Expansion is present
	// exit the app if i2c reads fail
	if (drv.checkInit() == false) {
		dbg.print(ONION_SEVERITY_FATAL, "> ERROR: OLED Expansion not found!\n");
		return 0;
	}


	// initialize display
	if ( init == 1 ) {
		drv.init();
		if (drv.checkInit() == false) {
			dbg.print(ONION_SEVERITY_FATAL, "main-oled-exp:: display init failed!\n");
		}
	}

	// clear screen
	if ( clear == 1 ) {
		dbg.print(ONION_SEVERITY_INFO, "> Clearing display\n");
		drv.clear();
		/*if (status == EXIT_FAILURE) {
			dbg.print(ONION_SEVERITY_FATAL, "main-oled-exp:: display clear failed!\n");
		}*/
	}


	// check if just option command
	if ( argc == 0 ) {
		// check if usage needs to be printed
		if ( init == 0 && clear == 0) {
			usage(progname);
		}
		return 0;
	}


	//// parse the command arguments
	while ( argc > 0 ) {
		if(strlen(argv[0]) > MAX_COMMAND_LENGTH || strlen(argv[1]) > MAX_PARAM_LENGTH) {
			// FIXME: This error needs rewording. Also, the exit status should be less funny.
			dbg.print(ONION_SEVERITY_FATAL, "Unsupported parameter length\n");
			exit(13);
		}
		
		// first arg - command
		strcpy(command, argv[0]);

		// second arg - parameter (optional)
		if ( argc > 1 ) {
			strcpy(param, argv[1]);
		}

		// perform the specified command
		status 	= command(command, param);
		if (status != EXIT_SUCCESS) {
			dbg.print(ONION_SEVERITY_FATAL, "ERROR: command '%s' failed!\n", command);
		}

		// decrement the number of arguments left
		argc	-= 2;
		argv	+= 2;

		dbg.print(ONION_SEVERITY_DEBUG, "> arguments remaining: %d\n", argc);
	}


	return 0;
}
