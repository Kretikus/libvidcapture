#pragma once

#include <memory>

class DSVideoDevice
{
public:
	DSVideoDevice();
	~DSVideoDevice();

	bool setMediaType();
	bool buildFilterGraph();
	bool addNullRenderer();

private:
	class Impl;
	std::shared_ptr<Impl> d_;
};