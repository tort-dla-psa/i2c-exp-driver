#ifndef _ONION_MCP23008_DRIVER_
#define _ONION_MCP23008_DRIVER_

#include <stdlib.h>
#include <stdio.h>

#include "onion-i2c.h"


#define MCP23008_I2C_DEVICE_NUM		(I2C_DEFAULT_ADAPTER)
#define MCP23008_I2C_DEVICE_ADDR	0x20

#define MCP23008_NUM_GPIOS			8

// define register addresses
#define MCP23008_REG_IODIR			0x00
#define MCP23008_REG_IPOL			0x01
#define MCP23008_REG_GPINTEN		0x02
#define MCP23008_REG_DEFVAL			0x03
#define MCP23008_REG_INTCON			0x04
#define MCP23008_REG_IOCON			0x05
#define MCP23008_REG_GPPU			0x06
#define MCP23008_REG_INTF			0x07
#define MCP23008_REG_INTCAP			0x08
#define MCP23008_REG_GPIO			0x09
#define MCP23008_REG_OLAT			0x0A


// note on device addr and hw:
//	device addr is 0x20 when A[2:0] pins are driven to 0
//
//	increment in binary when A[2:0] pins are driven:	
//		A[2:0]		device addr
//		000 		0x20
//		001 		0x21
//		010 		0x22
//		011 		0x23
//		100 		0x24
//		101 		0x25
//		110 		0x26
//		111 		0x27

class fastMCP23008Driver{
	fastI2CDriver drv;

	void set_bit(int* regVal, int bitNum, int value);
	int get_bit(int regVal, int bitNum);
	int _ValidateGpio(int gpio);
	void set_reg(int devAddr, int addr, int val);
	void read_reg(int devAddr, int addr, int* val);
	void set_reg_bit(int devAddr, int addr, int bitNum, int bitVal);
	void _SetAttribute(int devAddr, int gpio, int addr, int val);
	void _GetAttribute(int devAddr, int gpio, int addr, int* val);
public:
	fastMCP23008Driver();
	~fastMCP23008Driver();
	// helper functions
	// functions to set a single GPIO's attributes
	void mcp_setDirection(int devAddr, int gpio, int bInput);
	void mcp_setInputPolarity(int devAddr, int gpio, int bActiveLow);
	void mcp_setPullup(int devAddr, int gpio, int bPullUp);
	void mcp_setGpio(int devAddr, int gpio, int value);
	// functions to set attributes for all GPIOs
	//	expecting 8-bit values
	void mcp_setAllDirection(int devAddr, int bInput);
	void mcp_setAllInputPolarity(int devAddr, int bActiveLow);
	void mcp_setAllPullup(int devAddr, int bPullUp);
	void mcp_setAllGpio(int devAddr, int value);
	// functions to read a single GPIO's attributes
	void mcp_getDirection(int devAddr, int gpio, int* bInput);
	void mcp_getInputPolarity(int devAddr, int gpio, int* bActiveLow);
	void mcp_getPullup(int devAddr, int gpio, int* bPullUp);
	void mcp_getGpio(int devAddr, int gpio, int* value);
};

#endif // _ONION_MCP23008_DRIVER_ 
