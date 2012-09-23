#include <cstdio>
#include "VideoCapture.h"
#include "CaptureWindow.h"

#define TEST(x) printf("\nPress enter to test: " #x); getchar(); fflush(stdin); x;

CaptureWindow* cw;

void callback(unsigned char* data, int len, int bpp, VideoDevice* dev)
{
	int id = dev->GetId()-1;
	int x = 330 * (id%2) + 10;
	int y = 250 * (id/2) + 10;

	cw->DrawCapture(x,y,320,240,bpp,data);
}

int main()
{
	 cw = new CaptureWindow();
	
	VideoCapture* vc		= new VideoCapture();
	VideoDevice* devices	= vc->GetDevices();
	int num_devices			= vc->NumDevices();

	for (int i=0; i<num_devices; i++)
	{
		printf("%s\n", devices[i].GetFriendlyName());
		devices[i].SetCallback(callback);
		devices[i].Start();
	}
	cw->Show();

	return 0;
}