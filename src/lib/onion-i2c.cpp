#include <onion-i2c.h>

bool fastI2CDriver::_getFd(int adapterNum) {
	int 	status;
	char 	pathname[255];
	// define the path to open
	status = snprintf(pathname, sizeof(pathname), I2C_DEV_PATH, adapterNum);
	// check the filename	

	if (status < 0 || status >= sizeof(pathname)) {
		// add errno
		return write_possible = false;
	}
	// create a file descriptor for the I2C bus
#ifdef I2C_ENABLED
	fd = open(pathname, O_RDWR);
#ifdef DEBUG
	debuger.print(ONION_SEVERITY_DEBUG_EXTRA, "%s fd =  %d\n", I2C_PRINT_BANNER, fd);
#endif
#else
	fd = 0;
#endif
	// check the defvice handle
	if (fd < 0) {
		// add errno
		return write_possible = false;
	}
	return write_possible = true;
}

bool fastI2CDriver::_releaseFd() {
#ifdef I2C_ENABLED
	if (close(fd) < 0) {
		return false;
	}
#endif
	write_possible = false;
	return true;
}

bool fastI2CDriver::_setDevice(uint8_t addr) {
#ifdef I2C_ENABLED
	// set to 7-bit addr
	if (ioctl(fd, I2C_TENBIT, 0) < 0) {
		return write_possible = false;
	}
	// set the address
	if (ioctl(fd, I2C_SLAVE, addr) < 0) {
		return write_possible = false;
	}
#endif
	return write_possible = true;
}

bool fastI2CDriver::_setDevice10bit(uint8_t addr) {
#ifdef I2C_ENABLED
	// set to 10-bit addr
	if (ioctl(fd, I2C_TENBIT, 1) < 0) {
		return write_possible = false;
	}
	// set the address
	if (_setDevice(addr) != EXIT_SUCCESS) {
		return write_possible = false;
	}
#endif
	return write_possible = true;
}

bool fastI2CDriver::_write(const uint8_t * buffer, int size) {
	int 	index;
	// set the device address
#ifdef DEBUG
	debuger.print(ONION_SEVERITY_DEBUG_EXTRA, "%s writing buffer %d %d:\n", I2C_PRINT_BANNER, devNum, devAddr);
	for (index = 0; index < size; index++) {
		debuger.print(ONION_SEVERITY_DEBUG_EXTRA, "\tbuffer[%d]: 0x%02x\n", index, buffer[index]);
	}
#endif
#ifdef I2C_ENABLED
	// perform the write
	if (write_possible) {
		// write to the i2c device
		if (::write(fd, buffer, size) != size) {
#ifdef DEBUG
			debuger.print(ONION_SEVERITY_FATAL, "%s write issue for register 0x%02x, errno is %d: %s\n", I2C_PRINT_BANNER, buffer[0], errno, strerror(errno));
#endif
			return false;
		}
		return true;
	}
#endif
	return false;
}

fastI2CDriver::fastI2CDriver(int devNum, uint8_t devAddr)
	:devNum(devNum), devAddr(devAddr)
{
#ifdef DEBUG
	debuger = fastDebuger();
#endif
	_getFd(devNum);
	_setDevice(devAddr);
}

#ifdef DEBUG
fastI2CDriver::fastI2CDriver(int devNum, uint8_t devAddr, fastDebuger debuger)
	: devNum(devNum), devAddr(devAddr), debuger(debuger) 
{
	_getFd(devNum);
	_setDevice(devAddr);	
}
#endif

fastI2CDriver::fastI2CDriver(const fastI2CDriver & src) {
	this->devNum = src.getDevice();
	this->devAddr = src.getDevAddr();
	this->addr = src.getAddr();
#ifdef DEBUG
	this->debuger = src.getDebuger();
#endif
	this->fd = src.fd;
	_getFd(devNum);
	_setDevice(devAddr);	
	//debuger.print(ONION_SEVERITY_DEBUG_EXTRA, "%s constructor done, write:%d\n", I2C_PRINT_BANNER, (write_possible ? 1 : 0));
}

fastI2CDriver::~fastI2CDriver() {
	_releaseFd();
}
#ifdef DEBUG
void fastI2CDriver::setDebuger(fastDebuger debuger) {
	this->debuger = debuger;
}

 fastDebuger fastI2CDriver::getDebuger() const {
	return debuger;
}
#endif
 bool fastI2CDriver::setDevice(int devNum) {
	this->devNum = devNum;
	return _getFd(devNum);
}

 int fastI2CDriver::getDevice() const {
	return devNum;
}

 bool fastI2CDriver::setDevAddr(uint8_t devAddr) {
	this->devAddr = devAddr;
	return _setDevice(devAddr);
}

 uint8_t fastI2CDriver::getDevAddr() const {
	return devAddr;
}

void fastI2CDriver::setAddr(int addr) {
	this->addr = addr;
}

 uint8_t fastI2CDriver::getAddr() const {
	return addr;
}

bool fastI2CDriver::write(const uint8_t addr, const uint8_t * buffer, int size) {
	uint8_t *bufferNew;
	// allocate the new buffer
	size++;		// adding addr to buffer
	bufferNew = (uint8_t*)malloc(size * sizeof *bufferNew);
	// add the address to the data buffer
	bufferNew[0] = addr;
	memcpy(&bufferNew[1], &buffer[0], (size - 1) * sizeof *buffer);
	// perform the write
	bool status = _write(bufferNew, size);
	// free the allocated memory
	free(bufferNew);
	return status;
}

 bool fastI2CDriver::write(const uint8_t * buffer, int size) {
	return _write(buffer, size);
}

bool fastI2CDriver::write(uint8_t addr, int val) {
	int 	size, tmp, index;
	uint8_t	buffer[I2C_BUFFER_SIZE];
	//// buffer setup
	// clear the buffer
	memset(buffer, 0, sizeof(buffer));
	// push the address and data values into the buffer
	buffer[0] = (addr);
	buffer[1] = (val & 0xff);
	size = 2;
	// if value is more than 1-byte, add to the buffer
	tmp = (val >> 8);	// start with byte 1
	index = 2;
	while (tmp > 0x00) {
		buffer[index] = tmp & 0xff;
		tmp = tmp >> 8; // advance the tmp data by a byte
		index++; 		// increment the index
		size++;			// increase the size
	}
#ifdef DEBUG
	debuger.print(ONION_SEVERITY_DEBUG, "%s Writing to device 0x%02x: addr = 0x%02x, data = 0x%02x (data size: %d)\n", I2C_PRINT_BANNER, devAddr, addr, val, (size - 1));
#endif
	// write the buffer
	return write(buffer, size);
}

 bool fastI2CDriver::write(int val) {
	return write(addr,val);
}

bool fastI2CDriver::writeBytes(uint8_t addr, int val, int numBytes) {
	int 	size, index;
	uint8_t	buffer[I2C_BUFFER_SIZE];
	//// buffer setup
	// clear the buffer
	memset(buffer, 0, sizeof(buffer));
	// push the address and data values into the buffer
	buffer[0] = (addr);
	size = 1;
	// add all data bytes to buffer
	index = 1;
	for (index = 0; index < numBytes; index++) {
		buffer[index + 1] = (val >> (8 * index));
		size++;			// increase the size
	}
#ifdef DEBUG
	debuger.print(ONION_SEVERITY_DEBUG, "%s Writing to device 0x%02x: addr = 0x%02x, data = 0x%02x (data size: %d)\n", I2C_PRINT_BANNER, devAddr, addr, val, (size - 1));
#endif
	// write the buffer
	return _write(buffer, size);
}

 bool fastI2CDriver::writeBytes(int val, int numBytes) {
	return writeBytes(addr,val,numBytes);
}

bool fastI2CDriver::read(uint8_t addr, uint8_t * buffer, int numBytes) {
	int 	size, index;
	bool	status;
#ifdef DEBUG
	debuger.print(ONION_SEVERITY_DEBUG, "%s Reading %d byte%s from device 0x%02x: addr = 0x%02x", I2C_PRINT_BANNER, numBytes, (numBytes > 1 ? "s" : ""), devAddr, addr);
#endif
	// set the device address
	if (write_possible) {
		_setDevice(devAddr);
	}
	// perform the read
	if (write_possible) {
		//// set addr
		// clear the buffer
		memset(buffer, 0, numBytes);
		// push the address and data values into the buffer
		buffer[0] = addr;
		size = 1;
#ifdef I2C_ENABLED
		// write to the i2c device
		if (write(fd, buffer, size) != size) {
#ifdef DEBUG
			debuger.print(ONION_SEVERITY_FATAL, "%s write issue for register 0x%02x, errno is %d: %s\n", I2C_PRINT_BANNER, addr, errno, strerror(errno));
#endif
		}
#endif
		//// read data
		// clear the buffer
		memset(buffer, 0, numBytes);
#ifdef I2C_ENABLED
		// read from the i2c device
		size = numBytes;
		if (::read(fd, buffer, size) != size) {
#ifdef DEBUG
			debuger.print(ONION_SEVERITY_FATAL, "%s read issue for register 0x%02x, errno is %d: %s\n", I2C_PRINT_BANNER, addr, errno, strerror(errno));
#endif
			status = false;
		} else {
			status = true;
		}
#else
		buffer[0] = 0x0;
		size = 1;
#endif
		//// print the data
#ifdef DEBUG
		debuger.print(ONION_SEVERITY_DEBUG, "\tread %d byte%s, value: 0x", size, (size > 1 ? "s" : ""));
		for (index = (size - 1); index >= 0; index--) {
			debuger.print(ONION_SEVERITY_DEBUG, "%02x", (buffer[index] & 0xff));
		}
		debuger.print(ONION_SEVERITY_DEBUG, "\n");
#endif
	}
	// release the device file handle
	return status;
}
 bool fastI2CDriver::read(uint8_t * buffer, int numBytes) {
	return read(addr, buffer, numBytes);
}
bool fastI2CDriver::readByte(uint8_t addr, int * val) {
	bool 	status;
	uint8_t	buffer[I2C_BUFFER_SIZE];
	status = read(
		addr,
		buffer,
		1
	);
	*val = (int)(buffer[0]);
	return (status);
}

 bool fastI2CDriver::readByte(int * val) {
	return readByte(addr, val);
}

bool fastI2CDriver::operator==(const fastI2CDriver & rhs) {
	return (devNum==rhs.getDevice() &&
		devAddr==rhs.getDevAddr() &&
		fd==rhs.fd);
}