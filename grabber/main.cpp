#include <libvidcapture/vidcapture.h>

#include <iostream>

int main(int argc, char* argv[]) {
	(void)argc; (void)argv;
	using namespace vidcapture;

	VidCapture* capture = getVidCapture();
	if(!capture) return -1;

	auto devices = capture->getDevices();

	for(auto it = devices.begin(); it != devices.end(); ++it) {
		std::cout << (*it)->getName() << std::endl;
	}
	return 0;
}
