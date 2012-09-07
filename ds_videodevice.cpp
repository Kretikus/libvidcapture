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



/*
HRESULT WriteBitmap(PCWSTR, BITMAPINFOHEADER*, size_t, BYTE*, size_t);

HRESULT GrabVideoBitmap(PCWSTR pszVideoFile, PCWSTR pszBitmapFile)
{
    IGraphBuilder *pGraph = NULL;
    IMediaControl *pControl = NULL;
    IMediaEventEx *pEvent = NULL;
    IBaseFilter *pGrabberF = NULL;
    ISampleGrabber *pGrabber = NULL;
    IBaseFilter *pSourceF = NULL;
    IEnumPins *pEnum = NULL;
    IPin *pPin = NULL;
    IBaseFilter *pNullF = NULL;

    BYTE *pBuffer = NULL;

    HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, 
        CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&pGraph));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pGraph->QueryInterface(IID_PPV_ARGS(&pControl));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pGraph->QueryInterface(IID_PPV_ARGS(&pEvent));
    if (FAILED(hr))
    {
        goto done;
    }

    // Create the Sample Grabber filter.
    hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pGrabberF));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pGraph->AddFilter(pGrabberF, L"Sample Grabber");
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pGrabberF->QueryInterface(IID_PPV_ARGS(&pGrabber));
    if (FAILED(hr))
    {
        goto done;
    }

    AM_MEDIA_TYPE mt;
    ZeroMemory(&mt, sizeof(mt));
    mt.majortype = MEDIATYPE_Video;
    mt.subtype = MEDIASUBTYPE_RGB24;

    hr = pGrabber->SetMediaType(&mt);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pGraph->AddSourceFilter(pszVideoFile, L"Source", &pSourceF);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pSourceF->EnumPins(&pEnum);
    if (FAILED(hr))
    {
        goto done;
    }

    while (S_OK == pEnum->Next(1, &pPin, NULL))
    {
        hr = ConnectFilters(pGraph, pPin, pGrabberF);
        SafeRelease(&pPin);
        if (SUCCEEDED(hr))
        {
            break;
        }
    }

    if (FAILED(hr))
    {
        goto done;
    }

    hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, 
        IID_PPV_ARGS(&pNullF));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pGraph->AddFilter(pNullF, L"Null Filter");
    if (FAILED(hr))
    {
        goto done;
    }

    hr = ConnectFilters(pGraph, pGrabberF, pNullF);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pGrabber->SetOneShot(TRUE);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pGrabber->SetBufferSamples(TRUE);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pControl->Run();
    if (FAILED(hr))
    {
        goto done;
    }

    long evCode;
    hr = pEvent->WaitForCompletion(INFINITE, &evCode);

    // Find the required buffer size.
    long cbBuffer;
    hr = pGrabber->GetCurrentBuffer(&cbBuffer, NULL);
    if (FAILED(hr))
    {
        goto done;
    }

    pBuffer = (BYTE*)CoTaskMemAlloc(cbBuffer);
    if (!pBuffer) 
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = pGrabber->GetCurrentBuffer(&cbBuffer, (long*)pBuffer);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pGrabber->GetConnectedMediaType(&mt);
    if (FAILED(hr))
    {
        goto done;
    }

    // Examine the format block.
    if ((mt.formattype == FORMAT_VideoInfo) && 
        (mt.cbFormat >= sizeof(VIDEOINFOHEADER)) &&
        (mt.pbFormat != NULL)) 
    {
        VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)mt.pbFormat;

        hr = WriteBitmap(pszBitmapFile, &pVih->bmiHeader, 
            mt.cbFormat - SIZE_PREHEADER, pBuffer, cbBuffer);
    }
    else 
    {
        // Invalid format.
        hr = VFW_E_INVALIDMEDIATYPE; 
    }

    FreeMediaType(mt);

done:
    CoTaskMemFree(pBuffer);
    SafeRelease(&pPin);
    SafeRelease(&pEnum);
    SafeRelease(&pNullF);
    SafeRelease(&pSourceF);
    SafeRelease(&pGrabber);
    SafeRelease(&pGrabberF);
    SafeRelease(&pControl);
    SafeRelease(&pEvent);
    SafeRelease(&pGraph);
    return hr;
};

// Writes a bitmap file
//  pszFileName:  Output file name.
//  pBMI:         Bitmap format information (including pallete).
//  cbBMI:        Size of the BITMAPINFOHEADER, including palette, if present.
//  pData:        Pointer to the bitmap bits.
//  cbData        Size of the bitmap, in bytes.

HRESULT WriteBitmap(PCWSTR pszFileName, BITMAPINFOHEADER *pBMI, size_t cbBMI,
    BYTE *pData, size_t cbData)
{
    HANDLE hFile = CreateFile(pszFileName, GENERIC_WRITE, 0, NULL, 
        CREATE_ALWAYS, 0, NULL);
    if (hFile == NULL)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    BITMAPFILEHEADER bmf = { };

    bmf.bfType = 'MB';
    bmf.bfSize = cbBMI+ cbData + sizeof(bmf); 
    bmf.bfOffBits = sizeof(bmf) + cbBMI; 

    DWORD cbWritten = 0;
    BOOL result = WriteFile(hFile, &bmf, sizeof(bmf), &cbWritten, NULL);
    if (result)
    {
        result = WriteFile(hFile, pBMI, cbBMI, &cbWritten, NULL);
    }
    if (result)
    {
        result = WriteFile(hFile, pData, cbData, &cbWritten, NULL);
    }

    HRESULT hr = result ? S_OK : HRESULT_FROM_WIN32(GetLastError());

    CloseHandle(hFile);

    return hr;
}

*/