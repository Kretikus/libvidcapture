#include "ds_videodevice.h"

#include "ds_utils.h"

const int BITS_PER_PIXEL = 24;

class CallbackHandler : public ISampleGrabberCB
{
public:
	CallbackHandler(DSVideoDevice::Impl* parent);
	~CallbackHandler();

	void setCallback(VideoCaptureCallback cb);

	virtual HRESULT __stdcall SampleCB(double time, IMediaSample* sample);
	virtual HRESULT __stdcall BufferCB(double time, BYTE* buffer, long len);
	virtual HRESULT __stdcall QueryInterface( REFIID iid, LPVOID *ppv );
	virtual ULONG	__stdcall AddRef();
	virtual ULONG	__stdcall Release();

private:
	VideoCaptureCallback callback;
	DSVideoDevice::Impl* parent;
	ULONG cRef_;
};

class DSVideoDevice::Impl {
public:
	Impl(int id, const std::wstring & devName, IFilterGraph2* graph) 
		: id(id), filtername(devName), graph(graph),
		sourcefilter(), samplegrabberfilter(), nullrenderer(), samplegrabber()
	{
		callbackhandler = new CallbackHandler(this);
	}

	int             id;
	std::wstring    filtername;
	IFilterGraph2*  graph;

	IBaseFilter*    sourcefilter;
	IBaseFilter*    samplegrabberfilter;
	IBaseFilter*    nullrenderer;

	ISampleGrabber* samplegrabber;

	CallbackHandler* callbackhandler;
};

DSVideoDevice::DSVideoDevice(int id, const std::wstring & devName, IFilterGraph2* graph, ICaptureGraphBuilder2* capture, IMoniker* moniker)
: d_(new Impl(id, devName, graph))
{
	//add a filter for the device
	HRESULT hr = graph->AddSourceFilterForMoniker(moniker, 0, d_->filtername.c_str(), &d_->sourcefilter);
	if (hr != S_OK) throw hr;

	//create a samplegrabber filter for the device
	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER,IID_IBaseFilter,(void**)&d_->samplegrabberfilter);
	if (hr < 0) throw hr;

	//set mediatype on the samplegrabber
	hr = d_->samplegrabberfilter->QueryInterface(IID_ISampleGrabber, (void**)&d_->samplegrabber);
	if (hr != S_OK) throw hr;

	std::wstring sgFilterName = L"SG_" + d_->filtername;
	graph->AddFilter(d_->samplegrabberfilter, sgFilterName.c_str());

	//set the media type
	AM_MEDIA_TYPE mt;
	memset(&mt, 0, sizeof(AM_MEDIA_TYPE));

	mt.majortype = MEDIATYPE_Video;
	mt.subtype   = MEDIASUBTYPE_RGB24; 
	// setting the above to 32 bits fails consecutive Select for some reason
	// and only sends one single callback (flush from previous one ???)
	// must be deeper problem. 24 bpp seems to work fine for now.

	hr = d_->samplegrabber->SetMediaType(&mt);
	if (hr != S_OK) throw hr;

	//add the callback to the samplegrabber
	hr = d_->samplegrabber->SetCallback(d_->callbackhandler, 0);
	if (hr != S_OK) throw hr;

	//set the null renderer
	hr = CoCreateInstance(CLSID_NullRenderer,NULL,CLSCTX_INPROC_SERVER,IID_IBaseFilter,(void**) &d_->nullrenderer);
	if (hr < 0) throw hr; 

	std::wstring nrFilterName = L"NR_" + d_->filtername;
	graph->AddFilter(d_->nullrenderer, nrFilterName.c_str());

	//set the render path
#ifdef SHOW_DEBUG_RENDERER
	hr = capture->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, d_->sourcefilter, d_->samplegrabberfilter, NULL);
#else
	hr = capture->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, d_->sourcefilter, d_->samplegrabberfilter, d_->nullrenderer);
#endif
	if (hr < 0) throw hr; 

	#undef max // to get std::limits to work
	long long start=0;
	long long stop = std::numeric_limits<long long>::max();
	//if the stream is started, start capturing immediately
	hr = capture->ControlStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, d_->sourcefilter, &start, &stop, 1,2);
	if (hr < 0) throw hr;
}

int DSVideoDevice::getId() const
{
	return d_->id;
}

std::wstring DSVideoDevice::getName() const
{
	return d_->filtername;
}

void DSVideoDevice::setCallback(VideoCaptureCallback cb)
{
	d_->callbackhandler->setCallback(cb);
}

void DSVideoDevice::start()
{
	HRESULT hr;

	hr = d_->nullrenderer->Run(0);
	if (hr < 0) throw hr;

	hr = d_->samplegrabberfilter->Run(0);
	if (hr < 0) throw hr;

	hr = d_->sourcefilter->Run(0);
	if (hr < 0) throw hr;

}

void DSVideoDevice::stop()
{
	HRESULT hr;

	hr = d_->sourcefilter->Stop();
	if (hr < 0) throw hr;

	hr = d_->samplegrabberfilter->Stop();
	if (hr < 0) throw hr;

	hr = d_->nullrenderer->Stop();
	if (hr < 0) throw hr;

}


CallbackHandler::CallbackHandler(DSVideoDevice::Impl* parent)
: callback(), parent(parent), cRef_(0)
{
}
CallbackHandler::~CallbackHandler() {}

void CallbackHandler::setCallback(VideoCaptureCallback cb) { callback = cb; }
HRESULT __stdcall CallbackHandler::SampleCB(double time, IMediaSample* sample) 
{
	HRESULT hr;
	unsigned char* buffer;

	hr = sample->GetPointer((BYTE**)&buffer);
	if (hr != S_OK) return S_OK;

	//if (callback) callback(buffer, sample->GetActualDataLength(), BITS_PER_PIXEL, parent);
	return S_OK;
}

HRESULT __stdcall CallbackHandler::BufferCB(double time, BYTE* buffer, long len) {
	return S_OK;
}

HRESULT __stdcall CallbackHandler::QueryInterface( REFIID riid, LPVOID *ppvObject ) {

	if (NULL == ppvObject) return E_POINTER;
	if (riid == __uuidof(IUnknown))
	{
		*ppvObject = static_cast<IUnknown*>(this);
		return S_OK;
	}
	if (riid == __uuidof(ISampleGrabberCB))
	{
		*ppvObject = static_cast<ISampleGrabberCB*>(this);
		return S_OK;
	}
	return E_NOTIMPL;
}

ULONG __stdcall CallbackHandler::AddRef() {
	return ++cRef_;
}

ULONG __stdcall CallbackHandler::Release() {
	if (0 != --cRef_) return cRef_;
	delete this;
	return 0;
}
