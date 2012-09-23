#pragma once

#include "vidcapture.h"
#include <memory>

class Video4LinuxDevice;

namespace vidcapture {

class V4lVideoDevice : public VideoDevice
{
public:
	V4lVideoDevice(const std::string & fsDevName);
	~V4lVideoDevice();

	virtual bool                    isValid              () const;
	virtual std::string             getName              () const;
	virtual VideoDeviceCapabilities getDeviceCapabilities() const;

private:
	std::string        fsDevName_;
	std::shared_ptr<Video4LinuxDevice> device_;
};

class V4lVidCapture : public VidCapture
{
public:
	V4lVidCapture();
	~V4lVidCapture();

	virtual std::vector<VideoDevice*> getDevices();

private:
	std::vector<VideoDevice*> devices_;
};

}


