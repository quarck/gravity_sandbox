#include "stdafx.h"

#include <sstream>
#include <iomanip>
#include <stdint.h>
#include <vector>
#include <GL/gl.h>			/* OpenGL header file */
#include <GL/glu.h>			/* OpenGL utilities header file */

#include "PngLogger.h"

#include "lodepng.h"

PngLogger::PngLogger(const std::string& logFolder)
	: _logFolder{ logFolder }
	, _pixels{ 4 }
	, _pixelsFlipped { 4}
	, _vpWidth{ 1 }
	, _vpHeight{ 1 }
{
	::CreateDirectoryA(logFolder.c_str(), nullptr);
}


PngLogger::~PngLogger()
{
}

void PngLogger::onViewportResize(int width, int height)
{
	_vpWidth = width;
	_vpHeight = height;
	_pixels.resize(width * height * 4);
	_pixelsFlipped.resize(width * height * 4);
}

void PngLogger::onNewFrame()
{
	// Capture the actual pixels 
	glReadPixels(0, 0, _vpWidth, _vpHeight, GL_RGBA, GL_UNSIGNED_BYTE, &_pixels[0]);

	// BMP is a weird one, stored in a reverse order
	for (int row = 0; row < _vpHeight; ++row)
	{
		unsigned char* src_row = &_pixels[row * _vpWidth * 4]; // src img is RGBA
		unsigned char* dst_row  = &_pixelsFlipped[(_vpHeight-row-1) * _vpWidth * 4]; // src img is RGBA
		::memcpy(dst_row, src_row, _vpWidth * 4);
	}


	// todo: format the name with leading zeroes
	std::ostringstream str;
	str << _logFolder << "\\" << std::setw(8) << std::setfill('0') << _nextSeq++ << ".png";
	std::string name = str.str();

	lodepng::encode(name.c_str(), _pixelsFlipped.data(), _vpWidth, _vpHeight);
}
