#pragma once

#pragma comment(lib, "strmiids.lib")
#pragma warning(disable : 4995 )

#include <Dshow.h>

#include <assert.h>


#pragma region Formerly located in qedit.h in Windows SDK, now obsoleted and defined within project

struct __declspec(uuid("0579154a-2b53-4994-b0d0-e773148eff85"))
ISampleGrabberCB : IUnknown
{
	virtual HRESULT __stdcall SampleCB (double SampleTime, struct IMediaSample * pSample ) = 0;
	virtual HRESULT __stdcall BufferCB (double SampleTime, unsigned char * pBuffer, long BufferLen ) = 0;
};

struct __declspec(uuid("6b652fff-11fe-4fce-92ad-0266b5d7c78f"))
ISampleGrabber : IUnknown
{
	virtual HRESULT __stdcall SetOneShot (long OneShot ) = 0;
	virtual HRESULT __stdcall SetMediaType (struct _AMMediaType * pType ) = 0;
	virtual HRESULT __stdcall GetConnectedMediaType (struct _AMMediaType * pType ) = 0;
	virtual HRESULT __stdcall SetBufferSamples (long BufferThem ) = 0;
	virtual HRESULT __stdcall GetCurrentBuffer (/*[in,out]*/ long * pBufferSize, /*[out]*/ long * pBuffer ) = 0;
	virtual HRESULT __stdcall GetCurrentSample (/*[out,retval]*/ struct IMediaSample * * ppSample ) = 0;
	virtual HRESULT __stdcall SetCallback (struct ISampleGrabberCB * pCallback, long WhichMethodToCallback ) = 0;
};

struct __declspec(uuid("c1f400a0-3f08-11d3-9f0b-006008039e37")) SampleGrabber; // [ default ] interface ISampleGrabber

static const IID IID_ISampleGrabberCB  = { 0x0579154A, 0x2B53, 0x4994, { 0xB0, 0xD0, 0xE7, 0x73, 0x14, 0x8E, 0xFF, 0x85 } };
static const IID IID_ISampleGrabber    = { 0x6B652FFF, 0x11FE, 0x4fce, { 0x92, 0xAD, 0x02, 0x66, 0xB5, 0xD7, 0xC7, 0x8F } };
static const CLSID CLSID_SampleGrabber = { 0xC1F400A0, 0x3F08, 0x11d3, { 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37 } };
static const CLSID CLSID_NullRenderer  = { 0xC1F400A4, 0x3F08, 0x11d3, { 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37 } };

#pragma endregion



template <class T> void SafeRelease(T **ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

// Query whether a pin is connected to another pin.
//
// Note: This function does not return a pointer to the connected pin.
inline
HRESULT IsPinConnected(IPin *pPin, BOOL *pResult)
{
	IPin *pTmp = NULL;
	HRESULT hr = pPin->ConnectedTo(&pTmp);
	if (SUCCEEDED(hr))
	{
		*pResult = TRUE;
	}
	else if (hr == VFW_E_NOT_CONNECTED)
	{
		// The pin is not connected. This is not an error for our purposes.
		*pResult = FALSE;
		hr = S_OK;
	}

	SafeRelease(&pTmp);
	return hr;
}

// Query whether a pin has a specified direction (input / output)
inline
HRESULT IsPinDirection(IPin *pPin, PIN_DIRECTION dir, BOOL *pResult)
{
	PIN_DIRECTION pinDir;
	HRESULT hr = pPin->QueryDirection(&pinDir);
	if (SUCCEEDED(hr))
	{
		*pResult = (pinDir == dir);
	}
	return hr;
}


// Match a pin by pin direction and connection state.
inline
HRESULT MatchPin(IPin *pPin, PIN_DIRECTION direction, BOOL bShouldBeConnected, BOOL *pResult)
{
	assert(pResult != NULL);

	BOOL bMatch = FALSE;
	BOOL bIsConnected = FALSE;

	HRESULT hr = IsPinConnected(pPin, &bIsConnected);
	if (SUCCEEDED(hr))
	{
		if (bIsConnected == bShouldBeConnected)
		{
			hr = IsPinDirection(pPin, direction, &bMatch);
		}
	}

	if (SUCCEEDED(hr))
	{
		*pResult = bMatch;
	}
	return hr;
}

// Return the first unconnected input pin or output pin.
inline
HRESULT FindUnconnectedPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, IPin **ppPin)
{
	IEnumPins *pEnum = NULL;
	IPin *pPin = NULL;
	BOOL bFound = FALSE;

	HRESULT hr = pFilter->EnumPins(&pEnum);
	if (FAILED(hr))
	{
		goto done;
	}

	while (S_OK == pEnum->Next(1, &pPin, NULL))
	{
		hr = MatchPin(pPin, PinDir, FALSE, &bFound);
		if (FAILED(hr))
		{
			goto done;
		}
		if (bFound)
		{
			*ppPin = pPin;
			(*ppPin)->AddRef();
			break;
		}
		SafeRelease(&pPin);
	}

	if (!bFound)
	{
		hr = VFW_E_NOT_FOUND;
	}

done:
	SafeRelease(&pPin);
	SafeRelease(&pEnum);
	return hr;
}

inline
HRESULT ConnectFilters(
	IGraphBuilder *pGraph, // Filter Graph Manager.
	IPin *pOut,            // Output pin on the upstream filter.
	IBaseFilter *pDest)    // Downstream filter.
{
	IPin *pIn = NULL;

	// Find an input pin on the downstream filter.
	HRESULT hr = FindUnconnectedPin(pDest, PINDIR_INPUT, &pIn);
	if (SUCCEEDED(hr))
	{
		// Try to connect them.
		hr = pGraph->Connect(pOut, pIn);
		pIn->Release();
	}
	return hr;
}

// Connect filter to input pin.
inline
HRESULT ConnectFilters(IGraphBuilder *pGraph, IBaseFilter *pSrc, IPin *pIn)
{
	IPin *pOut = NULL;

	// Find an output pin on the upstream filter.
	HRESULT hr = FindUnconnectedPin(pSrc, PINDIR_OUTPUT, &pOut);
	if (SUCCEEDED(hr))
	{
		// Try to connect them.
		hr = pGraph->Connect(pOut, pIn);
		pOut->Release();
	}
	return hr;
}

// Connect filter to filter
inline
HRESULT ConnectFilters(IGraphBuilder *pGraph, IBaseFilter *pSrc, IBaseFilter *pDest)
{
	IPin *pOut = NULL;

	// Find an output pin on the first filter.
	HRESULT hr = FindUnconnectedPin(pSrc, PINDIR_OUTPUT, &pOut);
	if (SUCCEEDED(hr))
	{
		hr = ConnectFilters(pGraph, pOut, pDest);
		pOut->Release();
	}
	return hr;
}

template<typename T>
class CoInstance
{
public:
	CoInstance(const CLSID & clsid) : value_() {
		HRESULT hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&value_));
		if (FAILED(hr)) {
			value_ = 0;
		}
	}
	~CoInstance() {
		if(value_) value_->Release();
	}

	bool isValid() {
		return value_ != 0;
	}

	T* getInterface() { return value_; }

	IUnknown* getIUnknown() {
		return value_;
	}

public:
	T *value_;
};

template<typename T>
class InterfaceHolder {
public:
	InterfaceHolder(IUnknown * unknown) : value_()
	{
		if(!unknown) return;
		HRESULT hr = unknown->QueryInterface(IID_PPV_ARGS(&value_));
		if (FAILED(hr)) {
			value_ = 0;
		}
	}

	bool isValid() {
		return value_ != 0;
	}

	~InterfaceHolder() {
		if(value_) value_->Release();
		value_ = 0;
	}

	T* getInterface() { return value_; }

private:
	T* value_;
};