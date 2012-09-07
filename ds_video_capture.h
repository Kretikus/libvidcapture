#pragma once
#pragma comment(lib, "strmiids")

#define SKIP_DXTRANS
#define SHOW_DEBUG_RENDERER

/* If you are having the problem you can't open dxtrans.h:
 * Open qedit.h and add this to the start of the file:
 *
 * #ifdef SKIP_DXTRANS
 * #define __IDxtAlphaSetter_INTERFACE_DEFINED__
 * #define __IDxtJpeg_INTERFACE_DEFINED__
 * #define __IDxtKey_INTERFACE_DEFINED__
 * #define __IDxtCompositor_INTERFACE_DEFINED__
 * #endif
 *
 * Also replace the line 
 * #include "dxtrans.h"
 * with:
 * #ifndef SKIP_DXTRANS
 * #include "dxtrans.h"
 * #endif
 */

#include <windows.h>
#include <dshow.h>

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

static const IID IID_ISampleGrabberCB = { 0x0579154a, 0x2b53, 0x4994, { 0xb0, 0xd0, 0xe7, 0x73, 0x14, 0x8e, 0xff, 0x85 } };
static const IID IID_ISampleGrabber =   { 0x6B652FFF, 0x11FE, 0x4fce, { 0x92, 0xAD, 0x02, 0x66, 0xB5, 0xD7, 0xC7, 0x8F } };
static const CLSID CLSID_SampleGrabber = { 0xC1F400A0, 0x3F08, 0x11d3, { 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37 } };
static const CLSID CLSID_NullRenderer = { 0xC1F400A4, 0x3F08, 0x11d3, { 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37 } };

#pragma endregion


#ifndef MAXLONGLONG
#define MAXLONGLONG 0x7FFFFFFFFFFFFFFF
#endif

#ifndef MAX_DEVICES
#define MAX_DEVICES 8
#endif

#ifndef MAX_DEVICE_NAME
#define MAX_DEVICE_NAME 80
#endif

typedef struct
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
} RGB;

typedef void (*VideoCaptureCallback)(unsigned char* data, int len, int bitsperpixel);

class VideoDevice
{
public:
	VideoDevice();
	~VideoDevice();
	
	const char* GetFriendlyName();

private:
	char*			friendlyname;
	WCHAR*			filtername;
	IBaseFilter*	filter;
	friend class	VideoCapture;
};

class VideoCapture
{
public:
	VideoCapture(VideoCaptureCallback callback);
	~VideoCapture();

	VideoDevice* GetDevices();
	int  NumDevices();
	
	void Select(VideoDevice* dev);
	void Start();
	void Stop();

protected:
	void InitializeGraph();
	void SetSourceFilters();
	void SetSampleGrabber();
	void SetNullRenderer();
	void ConnectFilters();

private:
	IFilterGraph2*			graph;
	ICaptureGraphBuilder2*	capture;
	IMediaControl*			control;

	IBaseFilter*			nullrenderer;
	IBaseFilter*			samplegrabberfilter;
	ISampleGrabber*			samplegrabber;

	bool					playing;

	VideoDevice*			devices;
	VideoDevice*			current;

	int						num_devices;

	class CallbackHandler : public ISampleGrabberCB
	{
	public:
		CallbackHandler(VideoCaptureCallback cb);
		~CallbackHandler();

		void SetMediaType(AM_MEDIA_TYPE* am);

		virtual HRESULT __stdcall SampleCB(double time, IMediaSample* sample);
		virtual HRESULT __stdcall BufferCB(double time, BYTE* buffer, long len);
		virtual HRESULT __stdcall QueryInterface( REFIID iid, LPVOID *ppv );
		virtual ULONG	__stdcall AddRef();
		virtual ULONG	__stdcall Release();

	private:
		int						bitpixel;
		VideoCaptureCallback	callback;
		
	} *callbackhandler;
};