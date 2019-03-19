#include "relay-exp.h"

fastRelayDriver::fastRelayDriver()
{}
fastRelayDriver::~fastRelayDriver(){}

// perform basic initialization of GPIO chip
void fastRelayDriver::relayDriverInit(int addr) {
	//onionPrint(ONION_SEVERITY_INFO, "> Initializing Relay Expansion chip\n");
	// set all GPIOs to output
	mcp_drv.mcp_setAllDirection(addr, 0);
	//onionPrint(ONION_SEVERITY_FATAL, "relay-exp:init:: Setting output direction failed\n");
	// disable all pullup resistors
	mcp_drv.mcp_setAllPullup(addr, 0);
	//onionPrint(ONION_SEVERITY_FATAL, "relay-exp:init:: Disabling pullup resistors failed\n");
	// set all GPIOs to 0
	mcp_drv.mcp_setAllGpio(addr, 0);
	//onionPrint(ONION_SEVERITY_FATAL, "relay-exp:init:: Reseting GPIOs failed\n");
}

// check if GPIO0 and GPIO1 are properly initialized
void fastRelayDriver::relayCheckInit(int addr, int *bInitialized) {
	int dirGpio0, dirGpio1, puGpio0, puGpio1;
	// set default to not initialized
	*bInitialized	= 0;
	// find GPIO0 direction
	mcp_drv.mcp_getDirection(addr, RELAY_EXP_CHANNEL0, &dirGpio0);
	// find GPIO1 direction
	mcp_drv.mcp_getDirection(addr, RELAY_EXP_CHANNEL1, &dirGpio1);
	// find status of GPIO0's pull-up resistor
	mcp_drv.mcp_getInputPolarity(addr, RELAY_EXP_CHANNEL0, &puGpio0);
	// find status of GPIO1's pull-up resistor
	mcp_drv.mcp_getInputPolarity(addr, RELAY_EXP_CHANNEL1, &puGpio1);
	// check for any initialization
	if (dirGpio0 == 0 && 
		dirGpio1 == 0 && 
		puGpio0 == 0 && 
		puGpio1 == 0)
	{
		*bInitialized = 1;
	}
}

// read GPIO value, ie read the relay state
void fastRelayDriver::relayReadChannel(int addr, int channel, int* state) {
	// read the relay state
	mcp_drv.mcp_getGpio(addr, channel, state);
	//onionPrint(ONION_SEVERITY_INFO, "> Reading RELAY%d state: %s\n", channel, ( *state == 1 ? "ON" : "OFF") );
}

// set GPIO value - change the relay state
void fastRelayDriver::relaySetChannel(int addr, int channel, int state) {
	// set the relay channel to the specified state
	//onionPrint(ONION_SEVERITY_INFO, "> Setting RELAY%d to %s\n", channel, ( state == 1 ? "ON" : "OFF") );
	mcp_drv.mcp_setGpio(addr, channel, state);
}

// set value to both GPIO0 and fastRelayDriver::GPIO1(both relay states)
void fastRelayDriver::relaySetAllChannels(int addr, int state) {
	int val;
	//onionPrint(ONION_SEVERITY_INFO, "> Setting both RELAYS to %s\n", ( state == 1 ? "ON" : "OFF") );
	// translate state to register value
	if (state == 0) {
		val = 0x0;
	} else {
		val = 0x3;	// enable GPIO0 and GPIO1
	}
	// set the all relay channels to the specified state
	mcp_drv.mcp_setAllGpio(addr, val);
}
