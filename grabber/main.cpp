#include <libvidcapture/vidcapture.h>

#include "video_widget.h"

#include <iostream>

#include <QApplication>
#include <QImage>
#include <QDebug>

namespace {

template <typename T>
inline T clamp(T t, T low, T high) {
  return std::min(std::max(low, t), high);
}

template <typename T>
char clampByte(T x) {
  return static_cast<char>(clamp(static_cast<int>(x), 0, 255));
}

class RGBData {
public:
	RGBData(): r(), g(), b() {}
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

std::vector<std::vector<std::vector<RGBData> > > table;

void initTable()
{
	for(int y = 0; y < 256; ++y) {
		std::vector<std::vector<RGBData> > ydata;
		for(int u = 0; u < 256; ++u) {
			std::vector<RGBData> v;
			v.resize(256);
			ydata.push_back(v);
		}
		table.push_back(ydata);
	}

	for (int y = 0; y < 256; ++y) {
		for (int u = 0; u < 256; ++u) {
			for (int v = 0; v < 256; ++v) {
				RGBData d;
				d.r = clampByte(y + 1.402f * (v-128));
				d.g = clampByte(y - 0.34414f * (u-128) - 0.71414f * (v-128));
				d.b = clampByte(y + 1.772f * (y-128));
				table[y][u][v] = d;
			}
		}
	}
}

void clip(int & r, int & g, int & b) {
	if (r > 255) r = 255;
	if (g > 255) g = 255;
	if (b > 255) b = 255;
	if (r < 0) r = 0;
	if (g < 0) g = 0;
	if (b < 0) b = 0;
}

std::vector<unsigned char> convertYuv2RGB2(unsigned char* data, int len)
{
	std::vector<unsigned char> rgbData;

	const int width          = 640;
	const int height         = 480;
	const int bytesPerPixel  = 3;
	const int halfPixels     = width*height/2;

	rgbData.resize(width*height*bytesPerPixel, 0);

	unsigned char* srcDataPtr = data;
	unsigned char* dstDataPtr = &rgbData[0];
	for(int i = 0; i < halfPixels; ++i) {

		const int   Y1 = 0;
		const int   Y2 = 2;
		const int   Ca = 1;
		const int   Cb = 3;

		const RGBData& rgb1 = table[srcDataPtr[Y1]] [srcDataPtr[Ca]] [srcDataPtr[Cb]];
		dstDataPtr[0] = rgb1.r;
		dstDataPtr[1] = rgb1.g;
		dstDataPtr[2] = rgb1.b;

		const RGBData& rgb2 = table[srcDataPtr[Y2]] [srcDataPtr[Ca]] [srcDataPtr[Cb]];
		dstDataPtr[3] = rgb2.r;
		dstDataPtr[4] = rgb2.g;
		dstDataPtr[5] = rgb2.b;

		srcDataPtr += 4;
		dstDataPtr += 6;
	}
	return rgbData;
}
std::vector<unsigned char> convertYuv2RGB(unsigned char* data, int len)
{
	std::vector<unsigned char> rgbData;

	const int width          = 640;
	const int height         = 480;
	const int bytesPerPixel  = 3;
	const int halfPixels     = width*height/2;

	rgbData.resize(width*height*bytesPerPixel, 0);

	unsigned char* srcDataPtr = data;
	unsigned char* dstDataPtr = &rgbData[0];
	for(int i = 0; i < halfPixels; ++i) {

		const int   Y1 = 0;
		const int   Y2 = 2;
		const int   Ca = 1;
		const int   Cb = 3;
		const float r1 = 1.403;
		const float g1 = 0.344;
		const float g2 = 0.714;
		const float b1 = 1.770;
		const int   d  = 128;

		int r = srcDataPtr[Y1] + r1 * (srcDataPtr[Cb]-d);
		int g = srcDataPtr[Y1] - g1 * (srcDataPtr[Ca]-d) - g2 * (srcDataPtr[Cb]-d);
		int b = srcDataPtr[Y1] + b1 * (srcDataPtr[Ca]-d);

		clip(r,g,b);

		dstDataPtr[0] = r * 220 / 256;
		dstDataPtr[1] = g * 220 / 256;
		dstDataPtr[2] = b * 220 / 256;

		r = srcDataPtr[Y2] + r1 * (srcDataPtr[Cb]-d);
		g = srcDataPtr[Y2] - g1 * (srcDataPtr[Ca]-d) - g2 * (srcDataPtr[Cb]-d);
		b = srcDataPtr[Y2] + b1 * (srcDataPtr[Ca]-d);

		clip(r,g,b);

		dstDataPtr[3] = r * 220 / 256;
		dstDataPtr[4] = g * 220 / 256;
		dstDataPtr[5] = b * 220 / 256;

		srcDataPtr += 4;
		dstDataPtr += 6;
	}
	return rgbData;
}

} // end of anonymous namespace

void vidCallback(unsigned char* data, int len, int bitsperpixel, void* userDataPtr)
{
	//std::vector<unsigned char> rgbData = convertYuv2RGB2(data, len);

	VideoWidget* vw = reinterpret_cast<VideoWidget*>(userDataPtr);
	vw->setImage(QImage(data, 640,480, QImage::Format_RGB888));
	//vw->setImage(QImage(rgbData.data(), 640,480, QImage::Format_RGB888));
}


int main(int argc, char* argv[]) {
	using namespace vidcapture;

	QApplication app(argc, argv);
	//initTable();

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
