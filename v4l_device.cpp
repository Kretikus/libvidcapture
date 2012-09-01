#include "v4l_device.h"

#include <sys/types.h>
#include <dirent.h>
#include <libv4l2.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <errno.h>
#include <stdio.h>

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <sstream>

/// my gcc does not supply this C++11 (21.5) standard function yet. TODO: remove?
namespace std {
	string to_string(int value) {
		stringstream s; s << value;
		return s.str();
	}
}

typedef std::vector<std::string> StringList;

namespace {

StringList getVideoDevs() {
	StringList        ret;
	DIR*              dir = 0;
	struct dirent*    ent = 0;
	const std::string devDir("/dev/");

	dir = opendir(devDir.c_str());
	if (dir != 0) {
		while ((ent = readdir(dir)) != 0) {
			std::string entry(ent->d_name);
			if (entry.find("video") != std::string::npos) {
				ret.push_back(devDir + entry);
			}
		}
		closedir(dir);
	}
	return ret;
}

std::string getVersion(__u32 version) {
	uint major = (version >> 16) & 0x0F;
	uint minor = (version >>  8) & 0x0F;
	uint patch =  version        & 0xFF;
	return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
}

std::string printCapabilities(__u32 caps) {
	std::string ret;
	if(caps & V4L2_CAP_VIDEO_CAPTURE)        ret += "VideoCapture-SP ";
	if(caps & V4L2_CAP_VIDEO_CAPTURE_MPLANE) ret += "VideoCapture-MP ";
	if(caps & V4L2_CAP_VIDEO_OUTPUT)         ret += "VideoOutput-SP ";
	if(caps & V4L2_CAP_VIDEO_OUTPUT_MPLANE)  ret += "VideoOutput-SP ";
	//if(caps & V4L2_CAP_VIDEO_M2M)            ret += "VideoMemory-SP ";
	//if(caps & V4L2_CAP_VIDEO_M2M_MPLANE)     ret += "VideoMemory-MP ";
	if(caps & V4L2_CAP_VIDEO_OVERLAY)        ret += "VideoOverlay ";
	if(caps & V4L2_CAP_VBI_CAPTURE)          ret += "VideoCapture-VBI ";
	if(caps & V4L2_CAP_VBI_OUTPUT)           ret += "VideoOutput-VBI ";

	if(caps & V4L2_CAP_SLICED_VBI_CAPTURE)   ret += "VideoCapture-SlicedVBI ";
	if(caps & V4L2_CAP_SLICED_VBI_OUTPUT)    ret += "VideoOutput-SlicedVBI ";
	if(caps & V4L2_CAP_RDS_CAPTURE)          ret += "RDSCapture ";
	if(caps & V4L2_CAP_VIDEO_OUTPUT_OVERLAY) ret += "VideoOutputOverlay ";

	if(caps & V4L2_CAP_HW_FREQ_SEEK)         ret += "HW-Frequenzy ";
	if(caps & V4L2_CAP_RDS_OUTPUT)           ret += "RDSOutput ";
	if(caps & V4L2_CAP_TUNER)                ret += "Tuner ";

	if(caps & V4L2_CAP_AUDIO)                ret += "Audio ";
	if(caps & V4L2_CAP_RADIO)                ret += "Radio ";
	if(caps & V4L2_CAP_MODULATOR)            ret += "Modulator ";

	if(caps & V4L2_CAP_READWRITE)            ret += "ReadWrite ";
	if(caps & V4L2_CAP_ASYNCIO)              ret += "AsyncIo ";
	if(caps & V4L2_CAP_STREAMING)            ret += "Streaming ";
	if(caps & V4L2_CAP_DEVICE_CAPS)          ret += "DeviceCaps ";

	return ret;
}

std::string getFourcc(__u32 encodedCode)
{
	std::string fourcc;
	fourcc.resize(4);
	fourcc[3] = (encodedCode >> 24) & 0xFF;
	fourcc[2] = (encodedCode >> 16) & 0xFF;
	fourcc[1] = (encodedCode >>  8) & 0xFF;
	fourcc[0] =  encodedCode        & 0xFF;
	return fourcc;
}

__u32 v4l2Fourcc(char a, char b, char c, char d) {
	return ((__u32)(a) <<  0) |
		   ((__u32)(b) <<  8) |
		   ((__u32)(c) << 16) |
		   ((__u32)(d) << 24);
}

void printPixFormat(const v4l2_pix_format& pix) {
	std::cout << "Width:\t\t"        << pix.width << std::endl;
	std::cout << "Height:\t\t"       << pix.height << std::endl;
	std::cout << "PixelFormat:\t"    << getFourcc(pix.pixelformat) << std::endl;
	std::cout << "Bytes Per Line:\t" << pix.bytesperline << std::endl;
	std::cout << "Size image:\t\t"   << pix.sizeimage << std::endl;
	std::cout << "Colorspace:\t\t"   << pix.colorspace << std::endl << std::endl;
}

}

class Video4LinuxDevice {
public:
	Video4LinuxDevice(const std::string & devName) : devName_(devName), fd_()
	{
		std::memset(&caps_, 0, sizeof(caps_));
	}
	~Video4LinuxDevice() {
		if(fd_) close();
	}

	bool open() {
		fd_ = v4l2_open(devName_.c_str(), O_RDWR);
		if (fd_ == -1 ) {
			std::cout << "Could not open device. ErrNo: " << errno;
		}
		return fd_ != -1;
	}

	bool close() {
		v4l2_close(fd_);
		fd_ = 0;
		return true;
	}

	void capabilities() {
		int result = v4l2_ioctl(fd_, VIDIOC_QUERYCAP, &caps_);
		if (result == -1) {
			std::cout << "Could not query capabilities of the device. ErrNo: " << errno << std::endl;
			return;
		}
		std::cout << "Driver:\t " << caps_.driver << std::endl;
		std::cout << "Card:\t " << caps_.card << std::endl;
		std::cout << "Bus info:\t " << caps_.bus_info << std::endl;
		std::cout << "Version:\t " << getVersion(caps_.version) << std::endl;
		std::cout << "Total Caps:\t " << printCapabilities(caps_.capabilities) << std::endl;
		if(caps_.capabilities&V4L2_CAP_DEVICE_CAPS) {
			std::cout << "Dev Caps:\t " << printCapabilities(caps_.device_caps) << std::endl;
		}
	}

	void enumerateFrameSizes() {
		struct v4l2_frmsizeenum frameSz;
		std::memset(&frameSz, 0, sizeof(frameSz));
		int result = v4l2_ioctl(fd_, VIDIOC_ENUM_FRAMESIZES, &frameSz);
		if (result == -1) {
			std::cout << "Could not enumerate frame sizes. ErrNo: " << errno << std::endl;
			return;
		}
	}

	void enumerateFormats() {
		struct v4l2_fmtdesc desc;
		std::memset(&desc, 0, sizeof(desc));
		desc.pixelformat = v4l2Fourcc('Y','U', 'Y', 'V');
		desc.pixelformat = nativePixelFormat_;
		int result = 0;
		desc.index = 0;
		std::cout << std::endl << "Enumerating video formats:" << std::endl;
		while(result != -1) {
			result = v4l2_ioctl(fd_, VIDIOC_ENUM_FMT, &desc);
			if (result != -1) {
				std::cout
						  << desc.index << ": " << getFourcc(desc.pixelformat) << " - "
						  << desc.description << " (" << desc.flags << ")"
						  << std::endl;
				++desc.index;
			}
		}
		std::cout << "Finished emumerating video formats." << std::endl << std::endl;
	}

	void getCurrentFormat() {
		struct v4l2_format fmt;
		std::memset(&fmt, 0, sizeof(fmt));
		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		int result = v4l2_ioctl(fd_, VIDIOC_G_FMT, &fmt);
		if (result == -1) {
			std::cout << "Could not query current video format. ErrNo: " << errno;
			return;
		}
		v4l2_pix_format& pix = reinterpret_cast<v4l2_pix_format&>(fmt.fmt);
		printPixFormat(pix);
		nativePixelFormat_ = pix.pixelformat;
		pix.pixelformat = v4l2Fourcc('Y','U','Y','V');
		int result2 = v4l2_ioctl(fd_, VIDIOC_TRY_FMT, &fmt);
		if (result2 == -1) {
			std::cout << "Trying other format failed. ErrNo: " << errno;
		}
		printPixFormat(pix);
	}


private:
	std::string devName_;
	int fd_;
	__u32 nativePixelFormat_;
	struct v4l2_capability caps_;
};

V4lVideoDevice::V4lVideoDevice()
{
	StringList rawDevicesList = getVideoDevs();
	if (rawDevicesList.empty()) return;

	Video4LinuxDevice dev(rawDevicesList[0]);
	dev.open();
	dev.capabilities();
	dev.enumerateFormats();
	dev.getCurrentFormat();
	dev.enumerateFrameSizes();
}
