#include <libvidcapture/vidcapture.h>

#include <iostream>

int main(int argc, char* argv[]) {
	(void)argc; (void)argv;
	using namespace vidcapture;

	VidCapture* capture = getVidCapture();
	if(!capture) return -1;

	std::vector<VideoDevice*> devices = capture->getDevices();
	typedef std::vector<VideoDevice*>::const_iterator Iterator;

	for(Iterator it = devices.begin(); it != devices.end(); ++it) {
		std::cout << (*it)->getName() << std::endl;
	}
	return 0;
}
