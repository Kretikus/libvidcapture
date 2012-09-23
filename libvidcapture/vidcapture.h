#pragma once

#include <string>
#include <vector>

namespace vidcapture {

class PixelFormat {
public:
	enum Enum {
		Unknown = 0,
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

class VideoDeviceCapabilities {
public:
	VideoDeviceCapabilities() : width(), height() {}

public:
	unsigned int             width;
	unsigned int             height;
	PixelFormat              currentFormat;
	std::vector<PixelFormat> supportedFormats;
};

class VideoDevice {
public:
	virtual ~VideoDevice();

	virtual bool                    isValid              () const = 0;
	virtual std::string             getName              () const = 0;
	virtual VideoDeviceCapabilities getDeviceCapabilities() const = 0;
};

class VidCapture
{
public:
	virtual ~VidCapture();

	virtual std::vector<VideoDevice*> getDevices() = 0;
};


VidCapture* getVidCapture();

}
