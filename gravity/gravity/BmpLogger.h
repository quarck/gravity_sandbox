#pragma once
#include "IImageLogger.h"
#include <string>
#include <vector>

class BmpLogger : public IImageLogger
{
	std::string _logFolder;

	std::vector<unsigned char> _pixels;
	int _vpWidth;
	int _vpHeight;

	uint64_t _nextSeq{ 0 };

public:
	BmpLogger(const std::string& logFolder);

	virtual ~BmpLogger();

	void onViewportResize(int widht, int height) override;
	void onNewFrame() override;

};

