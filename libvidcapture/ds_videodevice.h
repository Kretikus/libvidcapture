#pragma once

#include "vidcapture.h"

#include <ObjIdl.h>
#include <strmif.h>

#include <string>
#include <memory>

namespace vidcapture {

class DSVideoDevice;

class DSVideoDevice : public VideoDevice {
public:
	DSVideoDevice(int id, const std::wstring & devName, IFilterGraph2* graph, ICaptureGraphBuilder2* capture, IMoniker*);

	virtual bool                    isValid              () const;
	virtual std::string             getName              () const;
	virtual VideoDeviceCapabilities getDeviceCapabilities() const;
	
	virtual void setCallback(VideoCallback cb, void* userDataPtr);
	virtual bool start();

	void stop();

protected:
	friend class CallbackHandler;
	class Impl;
	std::shared_ptr<Impl> d_;
};

}
