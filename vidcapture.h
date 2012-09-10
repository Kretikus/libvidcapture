#pragma once

#include <string>
#include <vector>

namespace vidcapture {

class PixelFormat {
public:
	enum Enum {
		Unknown,
		RGB24,
		YUV12,
		YUVY
	};

	PixelFormat(Enum v = Unknown) : value_(v) {}
	PixelFormat(int v) : value_(v) {}
	operator Enum() const { return static_cast<Enum>(value_); }

	std::string toString() const;

private:
	int value_;
};

class DeviceCapabilities {
public:
	DeviceCapabilities() : width(), height() {}

public:
	uint                     width;
	uint                     height;
	PixelFormat              currentFormat;
	std::vector<PixelFormat> supportedFormats;
};

class VideoDevice {
public:
	virtual ~VideoDevice();

	virtual bool               isValid              () const = 0;
	virtual std::string        getName              () const = 0;
	virtual DeviceCapabilities getDeviceCapabilities() const = 0;
};

class VidCapture
{
public:
	virtual ~VidCapture();

	virtual std::vector<VideoDevice*> getDevices() = 0;
};


VidCapture* getVidCapture();

}
