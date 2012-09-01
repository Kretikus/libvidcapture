
//#include <ds_videodevice.h>

#include <dshow.h>
#include <d3d9.h>
#include <vmr9.h>

#include <assert.h>
#include <vector>

HRESULT InitCaptureGraphBuilder(
	IGraphBuilder **ppGraph,  // Receives the pointer.
	ICaptureGraphBuilder2 **ppBuild  // Receives the pointer.
	)
{
	if (!ppGraph || !ppBuild)
	{
		return E_POINTER;
	}
	IGraphBuilder *pGraph = NULL;
	ICaptureGraphBuilder2 *pBuild = NULL;

	// Create the Capture Graph Builder.
	HRESULT hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, 
		CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void**)&pBuild );
	if (SUCCEEDED(hr))
	{
		// Create the Filter Graph Manager.
		hr = CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER,
			IID_IGraphBuilder, (void**)&pGraph);
		if (SUCCEEDED(hr))
		{
			// Initialize the Capture Graph Builder.
			pBuild->SetFiltergraph(pGraph);

			// Return both interface pointers to the caller.
			*ppBuild = pBuild;
			*ppGraph = pGraph; // The caller must release both interfaces.
			return S_OK;
		}
		else
		{
			pBuild->Release();
		}
	}
	return hr; // Failed
}


int main(int argc, char* argv[])
{
//	DSVideoDevice dv;
	CoInitialize(NULL);

	IGraphBuilder **pGraph,  // Receives the pointer.
	ICaptureGraphBuilder2

	IBaseFilter *vmr9ptr = NULL;
	HRESULT hr = CoCreateInstance(CLSID_VideoMixingRenderer9, 0, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&vmr9ptr);

	IVMRWindowlessControl9* controlPtr = 0;

	vmr9ptr->QueryInterface(IID_IVMRWindowlessControl9, (void**)controlPtr);
	assert ( controlPtr != 0 );

	// Get the current frame
	BYTE*   lpDib = 0;
	hr = controlPtr->GetCurrentImage(&lpDib);

	// If everything is okay, we can create a BMP
	if (SUCCEEDED(hr))
	{
		BITMAPINFOHEADER*   pBMIH = (BITMAPINFOHEADER*) lpDib;
		DWORD               bufSize = pBMIH->biSizeImage;

		// Let's create a bmp
		BITMAPFILEHEADER    bmpHdr;
		BITMAPINFOHEADER    bmpInfo;
		size_t              hdrSize     = sizeof(bmpHdr);
		size_t              infSize     = sizeof(bmpInfo);

		memset(&bmpHdr, 0, hdrSize);
		bmpHdr.bfType                   = ('M' << 8) | 'B';
		bmpHdr.bfOffBits                = static_cast<DWORD>(hdrSize + infSize);
		bmpHdr.bfSize                   = bmpHdr.bfOffBits + bufSize;

		// Builder the bit map info.
		memset(&bmpInfo, 0, infSize);
		bmpInfo.biSize                  = static_cast<DWORD>(infSize);
		bmpInfo.biWidth                 = pBMIH->biWidth;
		bmpInfo.biHeight                = pBMIH->biHeight;
		bmpInfo.biPlanes                = pBMIH->biPlanes;
		bmpInfo.biBitCount              = pBMIH->biBitCount;

		// boost::shared_arrays are awesome!
		//stdboost::shared_array<BYTE> buf(new BYTE[bmpHdr.bfSize]);//(lpDib);
		std::vector<BYTE> buf(bmpHdr.bfSize);
		memcpy(&buf[0],                       &bmpHdr,    hdrSize); // copy the header
		memcpy(&buf[0] + hdrSize,             &bmpInfo,   infSize); // now copy the info block
		memcpy(&buf[0] + bmpHdr.bfOffBits,    lpDib,      bufSize);

		// Do something with your image data ... seriously...
		CoTaskMemFree(lpDib);

	}

	return 0;
}