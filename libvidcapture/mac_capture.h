#pragma once

#include <memory>
#include <string>
#include <vector>

class MacDevice {
public:
	MacDevice();

	std::string getName() const;
	bool isValid() const;

private:
	friend class MacCapture;
	class Impl;
	Impl* d_;
};

class MacCapture
{
public:
	MacCapture();
	~MacCapture();

	std::vector<MacDevice> getDevices() const;

	void openDevice(const MacDevice & videoDevice);

private:
	class Impl;
	Impl* d_;
};
