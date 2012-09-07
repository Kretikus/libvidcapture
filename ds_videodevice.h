#pragma once

#include <ObjIdl.h>
#include <strmif.h>

#include <string>
#include <memory>

class DSVideoDevice;
typedef void (*VideoCaptureCallback)(unsigned char* data, int len, int bitsperpixel, DSVideoDevice* dev);

class DSVideoDevice {
public:
	DSVideoDevice(int id, const std::wstring & devName, IFilterGraph2* graph, ICaptureGraphBuilder2* capture, IMoniker*);

	int          getId()   const;
	std::wstring getName() const;

	void setCallback(VideoCaptureCallback cb);
	void start();
	void stop();

protected:
	friend class CallbackHandler;
	class Impl;
	std::shared_ptr<Impl> d_;
};

