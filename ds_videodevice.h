#pragma once

#include "vidcapture.h"

#include <ObjIdl.h>
#include <strmif.h>

#include <string>
#include <memory>

namespace vidcapture {

class DSVideoDevice;
typedef void (*VideoCaptureCallback)(unsigned char* data, int len, int bitsperpixel, DSVideoDevice* dev);

class DSVideoDevice : public VideoDevice {
public:
	DSVideoDevice(int id, const std::wstring & devName, IFilterGraph2* graph, ICaptureGraphBuilder2* capture, IMoniker*);

	virtual bool                    isValid              () const;
	virtual std::string             getName              () const;
	virtual VideoDeviceCapabilities getDeviceCapabilities() const;
	
	void setCallback(VideoCaptureCallback cb);
	void start();
	void stop();

protected:
	friend class CallbackHandler;
	class Impl;
	std::shared_ptr<Impl> d_;
};

}
