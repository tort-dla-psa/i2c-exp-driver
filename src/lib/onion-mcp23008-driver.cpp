#include "onion-mcp23008-driver.h"

fastMCP23008Driver::fastMCP23008Driver()
	:drv(MCP23008_I2C_DEVICE_NUM,MCP23008_I2C_DEVICE_ADDR)
{}
fastMCP23008Driver::~fastMCP23008Driver(){}

// change the value of a single bit
void fastMCP23008Driver::set_bit(int* regVal, int bitNum, int value) {
	(*regVal) ^= (-value ^ (*regVal)) & (1 << bitNum);
}

// find the value of a single bit
int fastMCP23008Driver::get_bit(int regVal, int bitNum) {
	// isolate the specific bit
	int value = ((regVal >> bitNum) & 0x1);
	return value;
}

// overwrite all values in register
void fastMCP23008Driver::set_reg(int devAddr, int addr, int val) {
	drv.setDevAddr(devAddr);
	drv.write(addr, val);
	//onionPrint(ONION_SEVERITY_FATAL, "mcp-driver:: writing value 0x%02x to addr 0x%02x failed\n", val, addr);
}

// read register value
void fastMCP23008Driver::read_reg(int devAddr, int addr, int* val) {
	drv.setDevAddr(devAddr);
	drv.readByte(addr, val);
	//onionPrint(ONION_SEVERITY_FATAL, "mcp-driver:: read from addr 0x%02x failed\n", addr);
}

// set only a single bit in a register
void fastMCP23008Driver::set_reg_bit(int devAddr, int addr, int bitNum, int bitVal) {
	int val;
	// read the value
	read_reg(devAddr, addr, &val);
	// inject the bit value
	set_bit(&val, bitNum, bitVal);
	// write the value
	set_reg(devAddr, addr, val);
}

// validate gpio number 
int fastMCP23008Driver::_ValidateGpio(int gpio) {
	int status = EXIT_FAILURE;
	if (gpio >= 0 && gpio < MCP23008_NUM_GPIOS) {
		status = EXIT_SUCCESS;
	}
	return status;
}

// function to generalize attribute setting functions
void fastMCP23008Driver::_SetAttribute(int devAddr, int gpio, int addr, int val) {
	int status;
	// validate the gpio number
	status	= _ValidateGpio(gpio);
	if (status == EXIT_FAILURE) {
		//TODO throw exception
		return;
	}

	// perform the register write to set the attribute
	set_reg_bit(devAddr, addr, gpio, val & 0x01);
}

// function to generalize attribute reading functions
void fastMCP23008Driver::_GetAttribute(int devAddr, int gpio, int addr, int* val) {
	int regVal;
	// validate the gpio number
	if (_ValidateGpio(gpio) == EXIT_FAILURE) {
		//TODO throw exception
		return;
	}
	// read the register
	read_reg(devAddr, addr, &regVal);
	// isolate the specific bit
	*val 	= get_bit(regVal, gpio);
}


// set attribute functions
void fastMCP23008Driver::mcp_setDirection(int devAddr, int gpio, int bInput) {
	_SetAttribute(devAddr, gpio, MCP23008_REG_IODIR, bInput);
}

void fastMCP23008Driver::mcp_setInputPolarity(int devAddr, int gpio, int bActiveLow) {
	_SetAttribute(devAddr, gpio, MCP23008_REG_IPOL, bActiveLow);
}

void fastMCP23008Driver::mcp_setPullup(int devAddr, int gpio, int bPullUp) {
	_SetAttribute(devAddr, gpio, MCP23008_REG_GPPU, bPullUp);
}

void fastMCP23008Driver::mcp_setGpio(int devAddr, int gpio, int value) {
	_SetAttribute(devAddr, gpio, MCP23008_REG_GPIO, value);
}


// functions to set attribute for all gpios
void fastMCP23008Driver::mcp_setAllDirection(int devAddr, int bInput) {
	set_reg(devAddr, MCP23008_REG_IODIR, bInput);
}

void fastMCP23008Driver::mcp_setAllInputPolarity(int devAddr, int bActiveLow) {
	set_reg(devAddr, MCP23008_REG_IPOL, bActiveLow);
}

void fastMCP23008Driver::mcp_setAllPullup(int devAddr, int bPullUp) {
	set_reg(devAddr, MCP23008_REG_GPPU, bPullUp);
}

void fastMCP23008Driver::mcp_setAllGpio(int devAddr, int value) {
	set_reg(devAddr, MCP23008_REG_GPIO, value);
}


// functions to read attribute for specific gpio
void fastMCP23008Driver::mcp_getDirection(int devAddr, int gpio, int* bInput) {
	_GetAttribute(devAddr, gpio, MCP23008_REG_IODIR, bInput);
}

void fastMCP23008Driver::mcp_getInputPolarity(int devAddr, int gpio, int* bActiveLow) {
	_GetAttribute(devAddr, gpio, MCP23008_REG_IPOL, bActiveLow);
}

void fastMCP23008Driver::mcp_getPullup(int devAddr, int gpio, int* bPullUp) {
	_GetAttribute(devAddr, gpio, MCP23008_REG_GPPU, bPullUp);
}

void fastMCP23008Driver::mcp_getGpio(int devAddr, int gpio, int* value) {
	_GetAttribute(devAddr, gpio, MCP23008_REG_GPIO, value);
}
