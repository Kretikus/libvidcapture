#include "ds_videocapture.h"

#include <iostream>

void callback(unsigned char* data, int len, int bpp) {}

int main(int argc, char* argv[]) {
	DSVideoCapture c;
	auto devices = c.getDevices();
	
	for(auto it = devices.begin(); it != devices.end(); ++it) {
		std::wcout << it->getName() << std::endl;
	}
}