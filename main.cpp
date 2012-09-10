#include "vidcapture.h"

#include <iostream>

int main(int argc, char* argv[])
{
	(void)argc; (void)argv;

	using namespace vidcapture;
	VidCapture* cap = getVidCapture();
	std::vector<VideoDevice*> devices = cap->getDevices();
	for(auto d: devices) {
		std::cout << d->getName() << std::endl;
	}
}
