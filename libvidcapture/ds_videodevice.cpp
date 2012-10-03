#include "ds_videodevice.h"

#include "ds_utils.h"

#include "util.h"

#define REGISTER_FILTERGRAPH

namespace vidcapture {

const int BITS_PER_PIXEL = 24;

#ifdef REGISTER_FILTERGRAPH

HRESULT addGraphToRot(IUnknown *pUnkGraph, DWORD *pdwRegister) 
{
	IMoniker * pMoniker;
	IRunningObjectTable *pROT;
	WCHAR wsz[128];
	HRESULT hr;

	if (!pUnkGraph || !pdwRegister)
		return E_POINTER;

	if (FAILED(GetRunningObjectTable(0, &pROT)))
		return E_FAIL;

	hr = StringCchPrintfW(wsz, NUMELMS(wsz), L"FilterGraph %08x pid %08x\0", (DWORD_PTR)pUnkGraph, 
		GetCurrentProcessId());

	hr = CreateItemMoniker(L"!", wsz, &pMoniker);
	if (SUCCEEDED(hr)) 
	{
		// Use the ROTFLAGS_REGISTRATIONKEEPSALIVE to ensure a strong reference
		// to the object.  Using this flag will cause the object to remain
		// registered until it is explicitly revoked with the Revoke() method.
		//
		// Not using this flag means that if GraphEdit remotely connects
		// to this graph and then GraphEdit exits, this object registration 
		// will be deleted, causing future attempts by GraphEdit to fail until
		// this application is restarted or until the graph is registered again.
		hr = pROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, pUnkGraph, 
			pMoniker, pdwRegister);
		pMoniker->Release();
	}

	pROT->Release();
	return hr;
}


// Removes a filter graph from the Running Object Table
void RemoveGraphFromRot(DWORD pdwRegister)
{
	IRunningObjectTable *pROT;

	if (SUCCEEDED(GetRunningObjectTable(0, &pROT))) 
	{
		pROT->Revoke(pdwRegister);
		pROT->Release();
	}
}

#endif


class CallbackHandler : public ISampleGrabberCB
{
public:
	CallbackHandler();
	~CallbackHandler();

	void setCallback(VideoCallback cb, void* userData);

	virtual HRESULT __stdcall SampleCB(double time, IMediaSample* sample);
	virtual HRESULT __stdcall BufferCB(double time, BYTE* buffer, long len);
	virtual HRESULT __stdcall QueryInterface( REFIID iid, LPVOID *ppv );
	virtual ULONG	__stdcall AddRef();
	virtual ULONG	__stdcall Release();

private:
	VideoCallback callback_;
	void* userData_;
	ULONG cRef_;
};

class DSVideoDevice::Impl {
public:
	Impl(int id, const std::wstring & devName, IFilterGraph2* graph)
		: id(id), filtername(devName), graph(graph),
		sourcefilter(), samplegrabberfilter(), nullrenderer(),
		samplegrabber(), callbackhandler(new CallbackHandler),
		dwGraphRegister()
	{}

	~Impl() {
		delete callbackhandler;
	}

	int              id;
	std::wstring     filtername;
	IFilterGraph2*   graph;

	IBaseFilter*     sourcefilter;
	IBaseFilter*     samplegrabberfilter;
	IBaseFilter*     nullrenderer;

	ISampleGrabber*  samplegrabber;

	CallbackHandler* callbackhandler;
	DWORD            dwGraphRegister;
};

DSVideoDevice::DSVideoDevice(int id, const std::wstring & devName, IFilterGraph2* graph, ICaptureGraphBuilder2* capture, IMoniker* moniker)
: d_(new Impl(id, devName, graph))
{
	//add a filter for the device
	HRESULT hr = graph->AddSourceFilterForMoniker(moniker, 0, d_->filtername.c_str(), &d_->sourcefilter);
	if (hr != S_OK) throw hr;

	//create a samplegrabber filter for the device
	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&d_->samplegrabberfilter);
	if (hr < 0) throw hr;

	//set mediatype on the samplegrabber
	hr = d_->samplegrabberfilter->QueryInterface(IID_ISampleGrabber, (void**)&d_->samplegrabber);
	if (hr != S_OK) throw hr;

	const std::wstring sgFilterName = L"SampleGrabberFilter_" + d_->filtername;
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
	hr = CoCreateInstance(CLSID_NullRenderer, NULL,CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**) &d_->nullrenderer);
	if (hr < 0) throw hr; 

	const std::wstring nrFilterName = L"NullRenderer_" + d_->filtername;
	graph->AddFilter(d_->nullrenderer, nrFilterName.c_str());
	
	//set the render path
#ifdef DEBUG_RENDERER
	hr = capture->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, d_->sourcefilter, d_->samplegrabberfilter, NULL);
#else
	hr = capture->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, d_->sourcefilter, d_->samplegrabberfilter, d_->nullrenderer);
#endif
	if (hr < 0) throw hr; 

	//#undef max // to get std::limits to work
	//long long start = 0;
	long long stop = MAXLONGLONG; //std::numeric_limits<long long>::max();
	//if the stream is started, start capturing immediately
	//hr = capture->ControlStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, d_->sourcefilter, &start, &stop, 1, 2);
	hr = capture->ControlStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, d_->sourcefilter, NULL, &stop, 1, 2);
	if (hr < 0) throw hr;


#ifdef REGISTER_FILTERGRAPH
	// Add our graph to the running object table, which will allow
	// the GraphEdit application to "spy" on our graph
	hr = addGraphToRot(d_->graph, &d_->dwGraphRegister);
	if (FAILED(hr))
	{
		//Msg(TEXT("Failed to register filter graph with ROT!  hr=0x%x"), hr);
		//g_dwGraphRegister = 0;
	}
#endif

}

bool DSVideoDevice::isValid() const
{
	return true;
}

VideoDeviceCapabilities DSVideoDevice::getDeviceCapabilities() const
{
	return VideoDeviceCapabilities();
}

std::string DSVideoDevice::getName() const
{
	//return d_->filtername;
	return StringConversion::toStdString(d_->filtername);
}

void DSVideoDevice::setCallback( VideoCallback cb, void* userDataPtr )
{
	d_->callbackhandler->setCallback(cb, userDataPtr);
}

bool DSVideoDevice::start()
{
	HRESULT hr;

	hr = d_->nullrenderer->Run(0);
	if (hr < 0) throw hr;

	hr = d_->samplegrabberfilter->Run(0);
	if (hr < 0) throw hr;

	hr = d_->sourcefilter->Run(0);
	if (hr < 0) throw hr;

	return true;
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


CallbackHandler::CallbackHandler()
: callback_(), userData_(), cRef_(0)
{
}

CallbackHandler::~CallbackHandler() {}

void CallbackHandler::setCallback(VideoCallback cb, void* userData) {
	callback_ = cb;
	userData_ = userData;
}

HRESULT __stdcall CallbackHandler::SampleCB(double time, IMediaSample* sample) 
{
	HRESULT hr;
	unsigned char* buffer;

	hr = sample->GetPointer((BYTE**)&buffer);
	if (hr != S_OK) return S_OK;

	if (callback_) callback_(buffer, sample->GetActualDataLength(), BITS_PER_PIXEL, userData_ );
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

}
