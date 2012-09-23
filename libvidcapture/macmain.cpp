#include "mac_capture.h"

#include <iostream>

int main(int argc, char* argv[])
{
	(void)argc; (void)argv;
	MacCapture mc;
	std::vector<MacDevice> devices = mc.getDevices();

	std::cout << "Displaying video devices:" << std::endl;
	for(std::vector<MacDevice>::const_iterator it = devices.begin(); it != devices.end(); ++it ) {
		std::cout << it->getName() << std::endl;
	}
}
