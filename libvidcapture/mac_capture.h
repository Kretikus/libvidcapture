#pragma once

#include "vidcapture.h"

#include <memory>
#include <string>
#include <vector>

namespace vidcapture {

class MacDevice : public VideoDevice
{
public:
	MacDevice();

	virtual bool                    isValid              () const;
	virtual std::string             getName              () const;
	virtual VideoDeviceCapabilities getDeviceCapabilities() const;

private:
	friend class MacCapture;
	class Impl;
	Impl* d_;
};

class MacCapture : public VidCapture
{
public:
	MacCapture();
	~MacCapture();

	virtual std::vector<VideoDevice*> getDevices();

	void openDevice(const MacDevice & videoDevice);

private:
	class Impl;
	Impl* d_;
};

}
