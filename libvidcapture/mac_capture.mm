#import "mac_capture.h"

#import <Cocoa/Cocoa.h>
#import <QTKit/QTKit.h>

@interface CaptureHelper : NSObject {
@public
		QTCaptureSession                 *mCaptureSession;
		QTCaptureDeviceInput             *mCaptureVideoDeviceInput;
		QTCaptureDecompressedVideoOutput *mCaptureDecompressedVideoOutput;
		CVImageBufferRef                  mCurrentImageBuffer;
		NSImage* image;
}

- (id)init;

- (void)captureOutput:   (QTCaptureOutput *)     captureOutput
	didOutputVideoFrame: (CVImageBufferRef)      videoFrame
	withSampleBuffer:    (QTSampleBuffer*)       sampleBuffer
	fromConnection:      (QTCaptureConnection *) connection;

- (void)addFrame:(NSImage*)outputImage;

- (void)open:(QTCaptureDevice*) videoDevice;

@end

@implementation CaptureHelper

-(id)init
{
	if ((self = [super init]))
	{
		mCaptureSession = [[QTCaptureSession alloc] init];
		NSSize s = NSMakeSize(0.0, 0.0);
		image = [ [NSImage alloc] initWithSize:s];
	}
	return self;
}

- (void)captureOutput: (QTCaptureOutput *)captureOutput
	didOutputVideoFrame: (CVImageBufferRef)videoFrame
	withSampleBuffer: (QTSampleBuffer*)sampleBuffer
	fromConnection: (QTCaptureConnection *)connection
{
	(void)captureOutput; (void)sampleBuffer; (void)connection;
	CVImageBufferRef imageBufferToRelease;
	CVBufferRetain(videoFrame);

	@synchronized (self) {
		imageBufferToRelease = mCurrentImageBuffer;
		mCurrentImageBuffer = videoFrame;
	}

	CVBufferRelease(imageBufferToRelease);
}

- (void)addFrame:(NSImage*)outputImage
{
	CVImageBufferRef imageBuffer;
	@synchronized(self) {
		imageBuffer = CVBufferRetain(mCurrentImageBuffer);
	}
	if( imageBuffer )
	{
		NSArray* reps = [outputImage representations];
		if ( [reps count] != 0 )
		{
			[outputImage removeRepresentation:[reps lastObject] ];
		}

		NSCIImageRep *imageRep = [NSCIImageRep imageRepWithCIImage:[CIImage imageWithCVImageBuffer:imageBuffer]];

		NSSize s = [outputImage size];
		NSSize t = [imageRep size];
		if( s.height != t.height || s.width != t.width ) {
				[outputImage setSize:[imageRep size]];
		}
		[outputImage addRepresentation:imageRep];
		CVBufferRelease(imageBuffer);
	}
}

- (void)open:(QTCaptureDevice*) videoDevice
{
	NSError *error;
	BOOL success = NO;

	mCaptureVideoDeviceInput = [[QTCaptureDeviceInput alloc] initWithDevice:videoDevice];

	success = [mCaptureSession addInput:mCaptureVideoDeviceInput error:&error];
	if (!success) {
		[[NSAlert alertWithError:error] runModal];
		return;
	}

	mCaptureDecompressedVideoOutput = [[QTCaptureDecompressedVideoOutput alloc] init];
	[mCaptureDecompressedVideoOutput setDelegate:self];
	//NSTimeInterval interval = 1/15;
	//[captureHelper->mCaptureDecompressedVideoOutput setMinimumVideoFrameInterval:interval];

	[mCaptureDecompressedVideoOutput setAutomaticallyDropsLateVideoFrames:YES];
	success = [mCaptureSession addOutput:mCaptureDecompressedVideoOutput error:&error];
	if (!success) {
		[[NSAlert alertWithError:error] runModal];
		return;
	}

	[mCaptureSession startRunning];
}

@end

namespace vidcapture {

class MacDevice::Impl {
public:
	Impl() : device() {}

	QTCaptureDevice *device;
};

MacDevice::MacDevice()
	: d_(new Impl)
{
}

std::string MacDevice::getName() const
{
	return std::string([[d_->device localizedDisplayName] UTF8String]);
}

bool MacDevice::isValid() const
{
	return d_->device;
}

VideoDeviceCapabilities MacDevice::getDeviceCapabilities() const
{
	return VideoDeviceCapabilities();
}


class MacCapture::Impl
{
public:
	Impl() : autoReleasePool(), captureHelper() {}
	~Impl()	{
		if(captureHelper && captureHelper->mCaptureSession) {
			[captureHelper->mCaptureSession stopRunning];
		}
	}

	NSAutoreleasePool* autoReleasePool;
	CaptureHelper* captureHelper;
};

MacCapture::MacCapture()
	: d_(new Impl())
{
	NSApplicationLoad();
	d_->autoReleasePool = [[NSAutoreleasePool alloc] init];
	d_->captureHelper = [[CaptureHelper alloc] init];
}


MacCapture::~MacCapture()
{
	delete d_;
}

std::vector<VideoDevice*> MacCapture::getDevices()
{
	std::vector<VideoDevice*> ret;
	//TODO check QTMediaTypeMuxed type as well

	//QTCaptureDevice *defaultDevice = [QTCaptureDevice defaultInputDeviceWithMediaType: QTMediaTypeVideo];
	//ret.push_back([[defaultDevice localizedDisplayName] UTF8String]);

	NSArray*  devicesWithMediaType = [QTCaptureDevice inputDevicesWithMediaType: QTMediaTypeVideo];

	NSEnumerator* enumerator = [devicesWithMediaType objectEnumerator];
	id value = 0;
	while ((value = [enumerator nextObject])) {
		MacDevice* dev = new MacDevice;
		dev->d_->device = (QTCaptureDevice*) value;
		ret.push_back(dev);
	}

	return ret;
}

void MacCapture::openDevice(const MacDevice & videoDevice)
{
	if (!videoDevice.isValid()) return;
	[d_->captureHelper open: videoDevice.d_->device];
}

}

vidcapture::VidCapture* vidcapture::getVidCapture()
{
	static vidcapture::VidCapture* ret = new MacCapture;
	return ret;
}

