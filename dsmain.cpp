#include "ds_video_capture.h"

#include <iostream>

void callback(unsigned char* data, int len, int bpp) {}

int main(int argc, char* argv[]) {
	VideoCapture * vc = new VideoCapture(callback);
	VideoDevice* devices = vc->GetDevices();
	int num_devices = vc->NumDevices();

	for(int i=0; i < num_devices; ++i) {
		std::cout << devices[i].GetFriendlyName() << std::endl;
	}
}