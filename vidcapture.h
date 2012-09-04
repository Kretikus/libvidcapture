#pragma once

#include <string>
#include <vector>

namespace vidcapture {

class PixelFormat {
public:
	enum type {
		RGB24,
		YUV12,
		YUVY
	};
};

class DeviceCapabilities {
public:
	DeviceCapabilities() : width(), height() {}

public:
	uint               width;
	uint               height;
	PixelFormat        currentFormat;
	std::vector<PixelFormat> supportedFormats;
};

class Device {
public:
	std::string name;
	int         deviceId;
};

class VidCapture
{
public:
	                            VidCapture() {}
	virtual                     ~VidCapture() {}
	
	virtual std::vector<Device> getDevices() = 0;
	virtual bool                open(const Device& device) = 0;
	virtual DeviceCapabilities  getCurrentCaps() = 0;
	
	// set format functions
	// set callback function
	// start/stop functions
};

}