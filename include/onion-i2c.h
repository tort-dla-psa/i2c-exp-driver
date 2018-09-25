#ifndef _ONION_I2C_H_
#define _ONION_I2C_H_

#include <stdlib.h>
#include <unistd.h>

#ifndef __APPLE__
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#endif

#include <sys/ioctl.h>
#include <fcntl.h>

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>

#include "onion-debug.h"

#define I2C_DEV_PATH		"/dev/i2c-%d"
#define I2C_PRINT_BANNER	"onion-i2c::"
#define I2C_BUFFER_SIZE		256
#define I2C_DEFAULT_ADAPTER	0

// for debugging
#ifndef __APPLE__
	#define I2C_ENABLED		1
#endif

class fastI2CDriver {
	bool 	_getFd(int adapterNum);
	bool 	_releaseFd();
	bool 	_setDevice(uint8_t addr);
	bool 	_setDevice10bit(uint8_t addr);
	bool 	_write(const uint8_t *buffer, int size);
	int		devNum, addr, fd;
	uint8_t devAddr;
	bool	write_possible;
	fastDebuger debuger;
public:
	fastI2CDriver(int devNum, uint8_t devAddr);
	fastI2CDriver(int devNum, uint8_t devAddr, fastDebuger debuger);
	fastI2CDriver(const fastI2CDriver &src);
	~fastI2CDriver();
	void	setDebuger(fastDebuger debuger);
	fastDebuger		getDebuger() const;
	bool	setDevice(int devNum);
	int		getDevice() const;
	bool	setDevAddr(uint8_t devAddr);
	uint8_t	getDevAddr() const;
	void	setAddr(int addr);
	uint8_t	getAddr() const;
	bool 	write(uint8_t addr, const uint8_t *buffer, int size);
	bool 	write(const uint8_t *buffer, int size);
	bool 	write(uint8_t addr, int val);
	bool 	write(int val);
	bool 	writeBytes(uint8_t addr, int val, int numBytes);
	bool 	writeBytes(int val, int numBytes);
	bool 	read(uint8_t addr, uint8_t *buffer, int numBytes);
	bool 	read(uint8_t *buffer, int numBytes);
	bool 	readByte(uint8_t addr, int *val);
	bool 	readByte(int *val);
	bool	operator == (const fastI2CDriver &rhs);
};
#endif // _ONION_I2C_H_