#include "ds_videodevice.h"

#include "ds_utils.h"

#include <string>


class SampleGrabberCallback : public ISampleGrabberCB
{
public:
	SampleGrabberCallback() : cRef_(0) {}

	// Fake reference counting.
	STDMETHODIMP_(ULONG) AddRef() { return ++cRef_; }
	STDMETHODIMP_(ULONG) Release() { 
		if (0 != --cRef_) return cRef_;
		delete this;
		return 0;
	}

	STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject)
	{
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

	STDMETHODIMP SampleCB(double Time, IMediaSample *pSample)
	{
		return E_NOTIMPL;
	}

	STDMETHODIMP BufferCB(double Time, BYTE *pBuffer, long BufferLen)
	{
		return E_NOTIMPL;
	}
private:
	ULONG cRef_;
};


class DSVideoDevice::Impl {
public:
	Impl() : pSourceF(), pEnum(), pPin(), pNullF() {}

	/// generic init
	std::shared_ptr<CoInstance<IGraphBuilder> > pGraph;
	std::shared_ptr<InterfaceHolder<IMediaControl> > pControl;
	std::shared_ptr<InterfaceHolder<IMediaEventEx> > pEvent;
	std::shared_ptr<CoInstance<IBaseFilter> > pGrabberF;
	std::shared_ptr<InterfaceHolder<ISampleGrabber> > pGrabber;

	/// for graph completion
	IBaseFilter* pSourceF;
	IEnumPins* pEnum;
	IPin* pPin;
	IBaseFilter* pNullF;
};


DSVideoDevice::DSVideoDevice()
: d_(new Impl)
{
	CoInitialize(NULL);

	d_->pGraph = std::make_shared<CoInstance<IGraphBuilder> >(CLSID_FilterGraph);
	d_->pControl = std::make_shared<InterfaceHolder<IMediaControl> >(d_->pGraph->getIUnknown());
	d_->pEvent = std::make_shared<InterfaceHolder<IMediaEventEx> >(d_->pGraph->getIUnknown());
	d_->pGrabberF = std::make_shared<CoInstance<IBaseFilter> >(CLSID_SampleGrabber);

	if(d_->pGrabberF->isValid()) {
		HRESULT hr = d_->pGraph->getInterface()->AddFilter(d_->pGrabberF->getInterface(), L"SampleGrabber" );
		if(SUCCEEDED(hr)) {
			d_->pGrabber = std::make_shared<InterfaceHolder<ISampleGrabber> >(d_->pGrabberF->getIUnknown());
		}
	}
}

DSVideoDevice::~DSVideoDevice()
{
	CoUninitialize();
}

bool DSVideoDevice::setMediaType()
{
	AM_MEDIA_TYPE mt;
	ZeroMemory(&mt, sizeof(mt));
	mt.majortype = MEDIATYPE_Video;
	mt.subtype = MEDIASUBTYPE_RGB24;

	HRESULT hr = d_->pGrabber->getInterface()->SetMediaType(&mt);
	if (FAILED(hr))
	{
		return false;
	}
	return true;
}

bool DSVideoDevice::buildFilterGraph()
{
	std::wstring szVideoFile = L"";
	HRESULT hr = d_->pGraph->getInterface()->AddSourceFilter(szVideoFile.c_str(), L"Source", &d_->pSourceF);
	if (FAILED(hr))
	{
		return false;
	}

	hr = d_->pSourceF->EnumPins(&d_->pEnum);
	if (FAILED(hr))
	{
		return false;
	}

	while (S_OK == d_->pEnum->Next(1, &d_->pPin, NULL))
	{
		hr = ConnectFilters(d_->pGraph->getInterface(), d_->pPin, d_->pGrabberF->getInterface());
		SafeRelease(&d_->pPin);
		if (SUCCEEDED(hr))
		{
			break;
		}
	}

	if (FAILED(hr))
	{
		return false;
	}
	return true;
}

bool DSVideoDevice::addNullRenderer()
{
	HRESULT hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&d_->pNullF));
	if (FAILED(hr))
	{
		return false;
	}

	hr = d_->pGraph->getInterface()->AddFilter(d_->pNullF, L"Null Filter");
	if (FAILED(hr))
	{
		return false;
	}

	hr = ConnectFilters(d_->pGraph->getInterface(), d_->pGrabberF->getInterface(), d_->pNullF);
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}
