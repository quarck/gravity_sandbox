#pragma once
#include "IImageLogger.h"
#include <string>
#include <vector>

class PngLogger : public IImageLogger
{
	std::string _logFolder;

	std::vector<unsigned char> _pixels;
	std::vector<unsigned char> _pixelsFlipped;
	int _vpWidth;
	int _vpHeight;

	uint64_t _nextSeq{ 0 };

public:
	PngLogger(const std::string& logFolder);

	virtual ~PngLogger();

	void onViewportResize(int widht, int height) override;
	void onNewFrame() override;

};

