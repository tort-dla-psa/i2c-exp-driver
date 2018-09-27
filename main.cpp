#include <iostream>
//#define DEBUG
//uncomment for debugging
//or pass -DDEBUG to g++

#include "include/onion-i2c.h"
#include "include/oled-exp.h"

int main() {
	std::cout << "Testing FastOledExp\n";
#ifdef DEBUG
	fastDebuger dbg = fastDebuger(ONION_VERBOSITY_EXTRA_VERBOSE);
	fastI2CDriver i2c_drv = fastI2CDriver(OLED_EXP_DEVICE_NUM, OLED_EXP_ADDR, dbg);
	fastOledDriver drv = fastOledDriver(i2c_drv,dbg);
#else
	fastI2CDriver i2c_drv = fastI2CDriver(OLED_EXP_DEVICE_NUM, OLED_EXP_ADDR);
	fastOledDriver drv = fastOledDriver(i2c_drv); 
#endif
	drv.init();
	if (!drv.checkInit()) {
		std::cout << "Failed init\n";
		return -1;
	}
	drv.setCursor(1, 0);
	drv.write("Hello world\n Hi there");
    return 0;
}