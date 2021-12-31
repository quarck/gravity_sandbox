#pragma once

#include <stdint.h>

class IImageLogger
{
public:
	virtual ~IImageLogger() {}

	virtual void onViewportResize(int widht, int height) = 0;
	virtual void onNewFrame(uint64_t seq) = 0;
};
