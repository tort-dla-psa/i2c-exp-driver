#include <string>
#include <sstream>
#include <iomanip>
#include <oled-exp.h>
#include <onion-debug.h>

fastDebuger dbg;
fastOledDriver drv;

// print the usage info 
void usage() {
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
int commandExec(const std::string &command, std::string param) {
	int 	val0, val1;
	uint8_t	*buffer;

	// perform the specified command
	dbg.print(ONION_SEVERITY_DEBUG_EXTRA, "command = '%s', param = '%s'\n",
			command.c_str(), param.c_str());
	if (command == "write") {	
		drv.write(param.c_str());
	}else if (command == "writeByte") {
		// parse the byte
		uint8_t byte;
		std::stringstream buff;
		if (param[0] == '0' && param[1] == 'x') {
			buff << std::setfill('0') << std::setw(2) << std::hex << param.substr(2);
		} else {
			buff << std::setfill('0') << std::setw(2) << std::hex << param;
		}
		buff >> byte;
		drv.sendData(byte);
	}else if (command == "brightness") {
		drv.setBrightness( std::atoi(param.c_str()) );
	}else if (command == "invert") {
		// interpret the parameter
		val0 	= 0;	// off by default
		if (param == "on") {
			val0 = 1;
		}
		drv.setDisplayMode( val0 );
	}else if (command == "power") {
		// interpret the parameter
		val0 	= 0;	// off by default
		if (param =="on") {
			val0 = 1;
		}
		drv.setDisplayPower(val0);
	}else if (command == "dim") {
		// interpret the parameter
		val0 	= 0;	// off by default
		if (param == "on") {
			val0 = 1;
		}
		drv.setDim(val0);
	}else if (command =="cursor") {
		// interpret the parameter
		std::stringstream ss(param);
		ss >> val0 >> val1;
		dbg.print(ONION_SEVERITY_INFO, "> Setting cursor to (%d, %d)\n", val0, val1);
		drv.setTextColumns();
		drv.setCursor(val0, val1);
	}else if (command == "cursorPixel") {
		// interpret the parameter
		std::stringstream ss(param);
		ss >> val0 >> val1;
		dbg.print(ONION_SEVERITY_INFO, "> Setting cursor to row: %d, pixel: %d\n", val0, val1);
		drv.setImageColumns();
		drv.setCursorByPixel(val0, val1);
	}else if (command == "draw") {
		// allocate memory for the buffer
		buffer 	= new uint8_t[OLED_EXP_WIDTH*OLED_EXP_HEIGHT/8];
		// FIXME: We should definitely do a #define for the buffer size calculation. This looks ugly.
		memset(buffer, 0, OLED_EXP_WIDTH*OLED_EXP_HEIGHT/8 * sizeof *buffer);
		// read the parameter
		std::string identifyer = std::string(OLED_EXP_READ_LCD_DATA_IDENTIFIER);
		if (param.compare(0,identifyer.length(), identifyer)){
			dbg.print(ONION_SEVERITY_INFO, "> Reading data from argument\n");
			dbg.print(ONION_SEVERITY_DEBUG_EXTRA, "  param length is %d\n", param.length() );
			// remove the data identifier from the string
			param = param.substr(identifyer.length());
			dbg.print(ONION_SEVERITY_DEBUG_EXTRA, "  after move: param length is %d\n", param.length() );
			// read the data into a buffer
			char* temp = (char*)param.c_str();
			drv.readLcdData(temp, buffer);
		}else {
			// read data from a file
			dbg.print(ONION_SEVERITY_INFO, "> Reading data from file '%s'\n", param.c_str());
			drv.readLcdFile(param.c_str(), buffer);
		}

		drv.draw(buffer, OLED_EXP_WIDTH*OLED_EXP_HEIGHT/8);
		/*}
		else {
			dbg.print(ONION_SEVERITY_FATAL, "ERROR: Cannot draw invalid data\n");
		}*/

		// deallocate memory for the buffer
		if (buffer != NULL) {
			dbg.print(ONION_SEVERITY_DEBUG_EXTRA, "> Deallocating buffer array\n");
			delete[] buffer;
		}
	}else if (command == "scroll") {
		// interpret the parameters
		val0 		= -1;
		val1 		= -1;
		if (param == "left") {
			val0 	= 0;	// horizontal scrolling
			val1	= 0;	// scrolling left
		}else if (param == "right") {
			val0 	= 0;	// horizontal scrolling
			val1	= 1;	// scrolling right
		}else if (param == "diagonal-left") {
			val0 	= 1;	// vertical scrolling
			val1	= 0;	// scrolling up
		}else if (param == "diagonal-right") {
			val0 	= 1;	// vertical scrolling
			val1	= 1;	// scrolling down
		}

		if (val0 == -1) {
			drv.scrollStop();
		}else if (val0 == 0) {
			// horizontal scrolling
			drv.scroll(val1, OLED_EXP_SCROLL_SPEED_5_FRAMES, 0, OLED_EXP_CHAR_ROWS-1);
		}else if (val0 == 1) {
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
	}else {
		dbg.print(ONION_SEVERITY_FATAL, "> Unrecognized command '%s'\n", command.c_str());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
	dbg = fastDebuger(0);
	drv = fastOledDriver();
	std::string command, param;
	int 	verbose;
	int 	init;
	int 	clear;
	int 	ch;

	// set the defaults
	init = 0;
	clear = 0;
	verbose = ONION_VERBOSITY_NORMAL;

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
				init = 1;
				break;
			case 'c':
				// perform clear sequence
				clear = 1;
				break;
			default:
				usage();
				return 0;
		}
	}

	// set the verbosity
	dbg.setVerbosity(verbose);

	// advance past the option arguments
	argc -= optind;
	argv += optind;

	// check if just option command
	if (argc == 0) {
		// check if usage needs to be printed
		if (init == 0 && clear == 0) {
			usage();
		}
		return 0;
	}

	//// OLED PROGRAMMING
	// check if OLED Expansion is present
	// exit the app if i2c reads fail

	if (init == 1) {
		drv.init();
		if (drv.checkInit() == false) {
			dbg.print(ONION_SEVERITY_FATAL, "main-oled-exp:: display init failed!\n");
		}
	} else {
		if (drv.checkInit() == false) {
			dbg.print(ONION_SEVERITY_FATAL, "> ERROR: OLED Expansion not found!\n");
			return 0;
		}
	}
	// initialize display

	// clear screen
	if (clear == 1) {
		dbg.print(ONION_SEVERITY_INFO, "> Clearing display\n");
		drv.clear();
	}

	//// parse the command arguments
	while (argc > 0) {
		if (strlen(argv[0]) > MAX_COMMAND_LENGTH || strlen(argv[1]) > MAX_PARAM_LENGTH) {
			// FIXME: This error needs rewording. Also, the exit status should be less funny.
			dbg.print(ONION_SEVERITY_FATAL, "Unsupported parameter length\n");
			exit(13);
		}

		// first arg - command
		command = std::string(argv[0]);

		// second arg - parameter (optional)
		if (argc > 1) {
			param = std::string(argv[1]);
		}

		// perform the specified command
		if (commandExec(command, param) != EXIT_SUCCESS) {
			dbg.print(ONION_SEVERITY_FATAL, "ERROR: command '%s' failed!\n", command.c_str());
		}
		// decrement the number of arguments left
		argc -= 2;
		argv += 2;
		dbg.print(ONION_SEVERITY_DEBUG, "> arguments remaining: %d\n", argc);
	}
	return 0;
}
