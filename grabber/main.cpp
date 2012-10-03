#include <libvidcapture/vidcapture.h>

#include "video_widget.h"

#include <iostream>

#include <QApplication>
#include <QImage>


void vidCallback(unsigned char* data, int len, int bitsperpixel, void* userDataPtr)
{
	VideoWidget* vw = reinterpret_cast<VideoWidget*>(userDataPtr);

	vw->setImage(QImage(data, 640,480, QImage::Format_RGB16));
}


int main(int argc, char* argv[]) {
	using namespace vidcapture;

	QApplication app(argc, argv);

	VidCapture* capture = getVidCapture();
	if(!capture) return -1;

	std::vector<VideoDevice*> devices = capture->getDevices();
	typedef std::vector<VideoDevice*>::const_iterator Iterator;

	for(Iterator it = devices.begin(); it != devices.end(); ++it) {
		std::cout << (*it)->getName() << std::endl;
	}

	VideoWidget vw;
	vw.show();

	VideoDevice* dev = *(devices.begin());

	dev->setCallback(vidCallback, &vw);
	dev->start();

	return app.exec();
}
