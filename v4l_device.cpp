#include "v4l_device.h"

#include <sys/mman.h>
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
#include <fstream>
#include <iomanip>

/// my gcc does not supply this C++11 (21.5) standard function yet. TODO: remove?
namespace std {
	string to_string(int value) {
		stringstream s; s << value;
		return s.str();
	}
}

typedef std::vector<std::string> StringList;

namespace {

struct BufferType  {
	void*  start;
	size_t length;
};

template<typename T>
inline
void clearMemory(T& t) {
	std::memset(&t, 0, sizeof(T));
}


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
		clearMemory(caps_);
		clearMemory(currFmt_);
	}
	~Video4LinuxDevice() {
		/* Cleanup. */
		for (uint i = 0; i < buffers_.size(); i++) {
			if (-1 == v4l2_munmap(buffers_[i].start, buffers_[i].length)) {
				std::cout << "Error unmapping buffer " << i << ". ErrNo: " << errno << std::endl;
			}
		}
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
		clearMemory(desc);
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
		clearMemory(fmt);
		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		int result = v4l2_ioctl(fd_, VIDIOC_G_FMT, &fmt);
		if (result == -1) {
			std::cout << "Could not query current video format. ErrNo: " << errno << std::endl;
			return;
		}
		v4l2_pix_format& pix = reinterpret_cast<v4l2_pix_format&>(fmt.fmt);
		printPixFormat(pix);
		nativePixelFormat_ = pix.pixelformat;
		pix.pixelformat = v4l2Fourcc('Y','U','Y','V');
		//pix.pixelformat = v4l2Fourcc('Y','U','1','2');
		int result2 = v4l2_ioctl(fd_, VIDIOC_TRY_FMT, &fmt);
		if (result2 == -1) {
			std::cout << "Trying other format failed. ErrNo: " << errno << std::endl;
			return;
		} else {
			std::cout << "Format reurned: " << errno << std::endl;
		}
		printPixFormat(pix);
		int result3 = v4l2_ioctl(fd_, VIDIOC_S_FMT, &fmt);
		if (result3 == -1) {
			std::cout << "Could not set other format. ErrNo: " << errno << std::endl;
			return;
		}
		currFmt_ = pix;
	}

	bool initMMapStreaming() {
		struct v4l2_requestbuffers reqbuf;
		clearMemory(reqbuf);
		reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		reqbuf.memory = V4L2_MEMORY_MMAP;
		reqbuf.count = 20;

		if (-1 == v4l2_ioctl(fd_, VIDIOC_REQBUFS, &reqbuf)) {
			if (errno == EINVAL) {
				std::cout << "Video capturing or mmap-streaming is not supported" << std::endl;
			} else {
				std::cout << "Init streaming failed. ErroNo: " << errno << std::endl;
			}
			return false;
		}

		/* We want at least five buffers. */
		if (reqbuf.count < 5) {
			/*TODO: You may need to free the buffers here. */
			std::cout << "Not enough buffer memory" << std::endl;
			return false;
		}
		buffers_.resize(reqbuf.count);

		for (uint i = 0; i < buffers_.size(); i++) {
			struct v4l2_buffer buffer;
			clearMemory(buffer);

			buffer.type = reqbuf.type;
			buffer.memory = V4L2_MEMORY_MMAP;
			buffer.index = i;
			if (-1 == v4l2_ioctl (fd_, VIDIOC_QUERYBUF, &buffer)) {
					std::cout << "VIDIOC_QUERYBUF failed" << std::endl;
					return false;
				}

				buffers_[i].length = buffer.length; /* remember for munmap() */

				buffers_[i].start = v4l2_mmap(NULL, buffer.length,
							PROT_READ | PROT_WRITE, /* recommended */
							MAP_SHARED,             /* recommended */
							fd_, buffer.m.offset);

				if (MAP_FAILED == buffers_[i].start) {
					/* TODO: If you do not exit here you should unmap() and free()
					   the buffers mapped so far. */
					std::cout << "Map failed for buffer " << i << std::endl;
				}
		}

		// finally queue them all!
		for (uint i = 0; i < buffers_.size(); i++) {
			struct v4l2_buffer buffer;
			clearMemory(buffer);
			buffer.type = reqbuf.type;
			buffer.memory = V4L2_MEMORY_MMAP;
			buffer.index = i;
			if(-1 == v4l2_ioctl(fd_, VIDIOC_QBUF, &buffer)) {
				std::cout << "Queueing of buffer " << i << " failed. ErroNo:" << errno << std::endl;
				return false;
			}
		}

		return true;
	}

	void initUPStreaming() {
		struct v4l2_requestbuffers reqbuf;
		clearMemory(reqbuf);
		reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		reqbuf.memory = V4L2_MEMORY_USERPTR;

		if (v4l2_ioctl (fd_, VIDIOC_REQBUFS, &reqbuf) == -1) {
			if (errno == EINVAL) {
				std::cout << "Video capturing or user pointer streaming is not supported" << std::endl;
			} else {
				std::cout << "VIDIOC_REQBUFS failed. ErrNo: " << errno << std::endl;
			}
		}
	}

	void streamFor5Seconds() {
		fd_set             fds;
		struct timeval     tv;
		struct v4l2_buffer buf;

		int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (-1 == v4l2_ioctl(fd_, VIDIOC_STREAMON, &type)) {
			std::cout << "Could not enable stream. ErrNo: " << errno << std::endl;
			return;
		}
		int r = 0;
		for (int i = 0; i < 20; i++) {
				do {
					FD_ZERO(&fds);
					FD_SET(fd_, &fds);

					/* Timeout. */
					tv.tv_sec = 2;
					tv.tv_usec = 0;

					r = select(fd_ + 1, &fds, NULL, NULL, &tv);
				} while ((r == -1 && (errno = EINTR)));
				
				if (r == -1) {
						std::cout << "Select failed. ErrNo: " << errno << std::endl;
						return;
				}
				clearMemory(buf);
				buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				buf.memory = V4L2_MEMORY_MMAP;
				v4l2_ioctl(fd_, VIDIOC_DQBUF, &buf);

				std::stringstream  outNameStream;
				outNameStream << "/home/roman/out" << std::setfill('0') <<  std::setw(3) << i << ".ppm";
				std::string fname = outNameStream.str();
				std::ofstream fout;
				fout.open(fname.c_str(), std::ios::out|std::ios::binary|std::ios::trunc);
				if (!fout.is_open()) {
						std::cout << "Cannot open image '" << fname << "'" << std::endl;
						return;
				}
				fout << "P6\n" << currFmt_.width << " " << currFmt_.height << " 255\n"; 
				fout.write(reinterpret_cast<const char*>(buffers_[buf.index].start), buf.bytesused);
				fout.close();
	
				v4l2_ioctl(fd_, VIDIOC_QBUF, &buf);
		}
	
		v4l2_ioctl(fd_, VIDIOC_STREAMOFF, &type);
	}

private:
	std::string devName_;
	int fd_;
	__u32 nativePixelFormat_;
	struct v4l2_capability caps_;
	v4l2_pix_format currFmt_;

	std::vector<BufferType> buffers_;
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
	//dev.enumerateFrameSizes();
	dev.initMMapStreaming();
	dev.streamFor5Seconds();

}
