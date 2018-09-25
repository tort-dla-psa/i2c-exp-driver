#include <iostream>
#include "include/oled-exp.h"

int main()
{
	std::cout << "Testing FastOledExp\n";
	fastOledDriver drv = fastOledDriver();
	drv.init();
	if (!drv.checkInit()) {
		std::cout << "Failed init\n";
		return -1;
	}
	drv.setCursor(1, 0);
	drv.write("Hello world\n Hi there");
    return 0;
}