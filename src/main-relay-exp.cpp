#include "relay-exp.h"
#include "onion-debug.h"

#define 	MAIN_RELAY_EXP_COMMAND_READ	1
#define 	MAIN_RELAY_EXP_COMMAND_WRITE	2

fastDebuger dbg;
fastRelayDriver drv;

void usage() {
	dbg.print(ONION_SEVERITY_FATAL, "\n");
	dbg.print(ONION_SEVERITY_FATAL, "Usage: relay-exp -i\n");
	dbg.print(ONION_SEVERITY_FATAL, "\n");
	dbg.print(ONION_SEVERITY_FATAL, "FUNCTIONALITY:\n");
	dbg.print(ONION_SEVERITY_FATAL, "\tJust initialize the Relay chip\n");
	dbg.print(ONION_SEVERITY_FATAL, "\n\n");
	dbg.print(ONION_SEVERITY_FATAL, "Usage: relay-exp [-qvi] [-s <bbb>] CHANNEL STATE\n");
	dbg.print(ONION_SEVERITY_FATAL, "\n");
	dbg.print(ONION_SEVERITY_FATAL, "CHANNEL is the specified Relay channel on the Expansion\n");
	dbg.print(ONION_SEVERITY_FATAL, "\tcan be: 0-1  to control a single channel\n");
	dbg.print(ONION_SEVERITY_FATAL, "\tcan be: 'all' to control all channels simultaneously\n");
	dbg.print(ONION_SEVERITY_FATAL, "STATE is the desired operational state of the relay\n");
	dbg.print(ONION_SEVERITY_FATAL, "\tcan be: 0  relay switch is OFF\n");
	dbg.print(ONION_SEVERITY_FATAL, "\tcan be: 1  relay switch is ON\n");
	dbg.print(ONION_SEVERITY_FATAL, "\n");
	dbg.print(ONION_SEVERITY_FATAL, "FUNCTIONALITY:\n");
	dbg.print(ONION_SEVERITY_FATAL, "\tProgram the CHANNEL to the specified relay state\n");
	dbg.print(ONION_SEVERITY_FATAL, "\n");
	dbg.print(ONION_SEVERITY_FATAL, "Usage: relay-exp [-qvi] [-s <bbb>] read CHANNEL\n");
	dbg.print(ONION_SEVERITY_FATAL, "\n");
	dbg.print(ONION_SEVERITY_FATAL, "CHANNEL is the specified Relay channel on the Expansion\n");
	dbg.print(ONION_SEVERITY_FATAL, "\tcan be: 0-1  to control a single channel\n");
	dbg.print(ONION_SEVERITY_FATAL, "\n");
	dbg.print(ONION_SEVERITY_FATAL, "FUNCTIONALITY:\n");
	dbg.print(ONION_SEVERITY_FATAL, "\tRead the state of the relay specified by the CHANNEL\n");
	dbg.print(ONION_SEVERITY_FATAL, "\n");
	dbg.print(ONION_SEVERITY_FATAL, "OPTIONS:\n");
	dbg.print(ONION_SEVERITY_FATAL, " -q 		  quiet: no output\n");
	dbg.print(ONION_SEVERITY_FATAL, " -v 		  verbose: lots of output\n");
	dbg.print(ONION_SEVERITY_FATAL, " -h 		  help: show this prompt\n");
	dbg.print(ONION_SEVERITY_FATAL, " -i 		  initialize the relay chip\n");
	dbg.print(ONION_SEVERITY_FATAL, " -s <bbb>	  dip-switch configuration in binary, not required if 000\n");
	dbg.print(ONION_SEVERITY_FATAL, " -a <addr>  relay expansion I2C address (mutually exclusive from -s option)\n");
	dbg.print(ONION_SEVERITY_FATAL, "\n");
}

// flip single char, check for valid input
int processSingleSwitch(char* out, char in) {
	int status 	= EXIT_FAILURE;

	if (in == '0') {
		*out 	= '1';
		status 	= EXIT_SUCCESS;
	} else if (in == '1') {
		*out 	= '0';
		status 	= EXIT_SUCCESS;
	}

	return status;
}

// process switch input into addr to be used by MCP chip
//	flip the bits (bit0=bit2, bit1=bit1, bit2=bit0)
// 	invert the bits
// 	convert to int
int processSwitchAddr(char* addrIn, int* addrOut) {
	char* 	output;
	int 	status;

	// check the input
	if (strlen(addrIn) != 3) {
		return EXIT_FAILURE;
	}

	// initialize the output
	output 	= (char*)malloc(sizeof(char) * 3);
	strcpy(output, RELAY_EXP_ADDR_SWITCH_DEFAULT_VAL);

	// perform the flip
	status 	= processSingleSwitch(&output[0], addrIn[2]);
	status 	|= processSingleSwitch(&output[1], addrIn[1]);
	status 	|= processSingleSwitch(&output[2], addrIn[0]);

	// convert the string to an integer
	*addrOut 	= (int)strtol(output, NULL, 2);

	// free the allocated memory
	free(output);

	return status;
}

int processAddrArgument(char* addrIn, int* addrOut) {
	int 	status 	= EXIT_FAILURE;
	int 	addr;

	if (strlen(addrIn) >= 2) {
		// parse the address
		if (addrIn[0] == '0' && addrIn[1] == 'x') {
			sscanf(addrIn, "0x%02x", &addr);
		} else {
			sscanf(addrIn, "%02x", &addr);
		}

		*addrOut 	= addr;
		status 		= EXIT_SUCCESS;
	}

	return status;
}

int checkFirstArgument (char* channelArgument, int *action) {
	// default action is write
	*action  = MAIN_RELAY_EXP_COMMAND_WRITE;

	if (strcmp(channelArgument, "read") == 0 ) {
		// action is read
		*action  = MAIN_RELAY_EXP_COMMAND_READ;
	}

	return EXIT_SUCCESS;
}

int readChannelArgument (char* channelArgument) {
	int channel 	= RELAY_EXP_NUM_CHANNELS;		// default value is invalid
	
	if (strcmp(channelArgument, "all") == 0 ) {
		channel 	= -1;	// all drivers
	} else {
		channel 	= (int)strtol(channelArgument, NULL, 10);
	}

	return channel;
}

int validateArguments(int channel, int state, int bExtended) {
	int status 			= EXIT_SUCCESS;
	int maxNumChannels 	= RELAY_EXP_NUM_CHANNELS_DEFAULT;

	if (bExtended == 1) {
		maxNumChannels 	= RELAY_EXP_NUM_CHANNELS_EXTENDED;
	}

	if (channel < -1 || channel >= maxNumChannels) {
		dbg.print(ONION_SEVERITY_FATAL, "ERROR: invalid CHANNEL selection\n");
		dbg.print(ONION_SEVERITY_FATAL, "Accepted values are:\n");
		dbg.print(ONION_SEVERITY_FATAL, "\t0-%d\n", maxNumChannels-1);
		dbg.print(ONION_SEVERITY_FATAL, "\tall\n");
		dbg.print(ONION_SEVERITY_FATAL, "\n");

		status = EXIT_FAILURE;
	}

	if (state != 0 && state != 1) {
		dbg.print(ONION_SEVERITY_FATAL, "ERROR: invalid STATE selection\n");
		dbg.print(ONION_SEVERITY_FATAL, "Accepted values are:\n");
		dbg.print(ONION_SEVERITY_FATAL, "\t0 or 1\n");
		dbg.print(ONION_SEVERITY_FATAL, "\n");

		status = EXIT_FAILURE;
	}

	return status;
}


int main(int argc, char** argv) {
	const char *progname;
	char 	*switchAddr;
	char 	*deviceAddr 	= NULL;
	
	int 	status;
	int 	verbose;
	int 	init;
	int 	ch;

	int 	programAction;
	int 	channel;
	int 	relayState;

	int 	devAddr;
	int 	bInitialized;
	int 	bExtended;

	// set defaults
	init 		= 0;
	verbose 	= ONION_VERBOSITY_NORMAL;
	bExtended 	= 0;

	switchAddr 	= new char[RELAY_EXP_ADDR_SWITCH_NUM];
	strcpy(switchAddr, RELAY_EXP_ADDR_SWITCH_DEFAULT_VAL);

	// save the program name
	progname 	= argv[0];	

	//// parse the option arguments
	while ((ch = getopt(argc, argv, "vqhies:a:")) != -1) {
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
		case 'e':
			// enable the extended channels
			bExtended 	= 1;
			break;
		case 's':
			// capture binary 
			strcpy (switchAddr, optarg);
			break;
		case 'a':
			// capture the address
			deviceAddr = new char[strlen(optarg)];
			strncpy (deviceAddr, optarg, strlen(optarg) );
			break;
		default:
			usage();
			return 0;
		}
	}

	// set the verbosity
	dbg.setVerbosity(verbose);

	// advance past the option arguments
	argc 	-= optind;
	argv	+= optind;

	// process the switch address
	status = processSwitchAddr(switchAddr, &devAddr);
	if (status == EXIT_FAILURE) {
		usage();
		dbg.print(ONION_SEVERITY_FATAL, "ERROR: invalid switch address argument!\n");
		return 0;
	}
	if (strcmp(switchAddr, RELAY_EXP_ADDR_SWITCH_DEFAULT_VAL) != 0) {
		dbg.print(ONION_SEVERITY_INFO, "> Switch configuration: %s\n", switchAddr);
	}

	// process the address argument (overrides the switch address argument)
	if (deviceAddr != NULL) {
		status = processAddrArgument(deviceAddr, &devAddr);

		if (devAddr != RELAY_EXP_ADDR_DEFAULT) {
			dbg.print(ONION_SEVERITY_INFO, "> Using device address: 0x%02x\n", devAddr);
		}
		// the mcp23008 function expect an offset from the default 0x20 address
		devAddr -= MCP23008_I2C_DEVICE_ADDR;

		// clean-up
		free(deviceAddr);
	}


	// check if just initialization
	if ( argc == 0 && init == 1 ) {
		drv.relayDriverInit(devAddr);
		//dbg.print(ONION_SEVERITY_FATAL, "main-relay-exp:: relay init failed!\n");
		return 0;
	}

	// ensure correct number of arguments
	if ( argc != 2) {
		usage();
		dbg.print(ONION_SEVERITY_FATAL, "ERROR: invalid amount of arguments!\n");
		return 0;
	}


	//// parse the arguments
	// find if reading or writing state
	checkFirstArgument(argv[0], &programAction);

	if (programAction == MAIN_RELAY_EXP_COMMAND_WRITE) {
		// first arg - channel
		channel 	= readChannelArgument(argv[0]);

		// second arg - relay state (on or off)
		if(strcmp(argv[1], "off") == 0 ||
			strcmp(argv[1], "Off") == 0 ||
			strcmp(argv[1], "OFF") == 0) 
		{
			relayState 	= 0;
		} else if (strcmp(argv[1], "on") == 0 ||
			strcmp(argv[1], "On") == 0 ||
			strcmp(argv[1], "ON") == 0) 
		{
			relayState 	= 1;
		} else {
			relayState 	= (int)strtol(argv[1], NULL, 10);
		}

		// validate the arguments
		status 	= validateArguments(channel, relayState, bExtended);
		if (status == EXIT_FAILURE) {
			return 0;
		}
	} else if (programAction == MAIN_RELAY_EXP_COMMAND_READ) {
		// first arg - channel
		channel 	= readChannelArgument(argv[1]);
	}


	//// RELAY PROGRAMMING
	// check if initialized
	drv.relayCheckInit(devAddr, &bInitialized);

	// exit the app if i2c reads fail
	//	dbg.print(ONION_SEVERITY_FATAL, "> ERROR: Relay Expansion not found!\n");

	// perform initialization
	if (init == 1 || bInitialized == 0) {
		drv.relayDriverInit(devAddr);
		//dbg.print(ONION_SEVERITY_FATAL, "main-relay-exp:: relay init failed!\n");
	}

	// set the relay state
	if (programAction == MAIN_RELAY_EXP_COMMAND_WRITE) {
		if (channel < 0) {
			// program both relays at once
			drv.relaySetAllChannels(devAddr, relayState);
			//dbg.print(ONION_SEVERITY_FATAL, "main-relay-exp:: all relay setup failed!\n");
		} else {
			// program just one relay
			drv.relaySetChannel(devAddr, channel, relayState);
			//dbg.print(ONION_SEVERITY_FATAL, "main-relay-exp:: relay %d setup failed!\n", channel);
		}
	} else if (programAction == MAIN_RELAY_EXP_COMMAND_READ) {
		drv.relayReadChannel(devAddr, channel, &relayState);
		//dbg.print(ONION_SEVERITY_FATAL, "main-relay-exp:: reading relay %d state failed!\n", channel);
	}

	// clean-up
	free(switchAddr);
	return 0;
}

