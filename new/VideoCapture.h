#pragma once
#pragma comment(lib, "strmiids")

#define SKIP_DXTRANS
//#define SHOW_DEBUG_RENDERER
#define _CRT_SECURE_NO_WARNINGS

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
#include <qedit.h>

#ifndef MAXLONGLONG
#define MAXLONGLONG 0x7FFFFFFFFFFFFFFF
#endif

#ifndef MAX_DEVICES
#define MAX_DEVICES 8
#endif

#ifndef MAX_DEVICE_NAME
#define MAX_DEVICE_NAME 80
#endif

#ifndef BITS_PER_PIXEL
#define BITS_PER_PIXEL 24
#endif

typedef struct
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
} RGB;

class VideoDevice;

typedef void (*VideoCaptureCallback)(unsigned char* data, int len, int bitsperpixel, VideoDevice* dev);

class VideoDevice
{
public:
	VideoDevice();
	~VideoDevice();
	
	int				GetId();
	const char*		GetFriendlyName();
	void			SetCallback(VideoCaptureCallback cb);

	void			Start();
	void			Stop();

private:
	int				id;
	char*			friendlyname;
	WCHAR*			filtername;
	
	IBaseFilter*	sourcefilter;
	IBaseFilter*	samplegrabberfilter;
	IBaseFilter*	nullrenderer;

	ISampleGrabber* samplegrabber;

	IFilterGraph2*	graph;

	class CallbackHandler : public ISampleGrabberCB
	{
	public:
		CallbackHandler(VideoDevice* parent);
		~CallbackHandler();

		void SetCallback(VideoCaptureCallback cb);

		virtual HRESULT __stdcall SampleCB(double time, IMediaSample* sample);
		virtual HRESULT __stdcall BufferCB(double time, BYTE* buffer, long len);
		virtual HRESULT __stdcall QueryInterface( REFIID iid, LPVOID *ppv );
		virtual ULONG	__stdcall AddRef();
		virtual ULONG	__stdcall Release();

	private:
		VideoCaptureCallback	callback;
		VideoDevice*			parent;

	} *callbackhandler;

	friend class	VideoCapture;
};

class VideoCapture
{
public:
	VideoCapture();
	~VideoCapture();

	VideoDevice* GetDevices();
	int  NumDevices();

protected:
	void InitializeGraph();
	void InitializeVideo();

private:
	IFilterGraph2*			graph;
	ICaptureGraphBuilder2*	capture;
	IMediaControl*			control;

	bool					playing;

	VideoDevice*			devices;
	VideoDevice*			current;

	int						num_devices;
};

