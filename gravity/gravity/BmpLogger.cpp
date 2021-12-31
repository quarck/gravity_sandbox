#include "stdafx.h"
#include "BmpLogger.h"
#include <sstream>
#include <iomanip>
#include <stdint.h>
#include <vector>
#include <GL/gl.h>			/* OpenGL header file */
#include <GL/glu.h>			/* OpenGL utilities header file */

// nested header structure decl
#pragma pack(push, 1)
struct BmpHeader
{
	uint8_t	signature[2];		// $00-$01  ASCII 2-byte "BM" bitmap identifier. 
	uint32_t	length;			// $02-$05  Total length of bitmap file in bytes. 
	uint32_t	reseved1;		// $06-$09  Reserved, possibly for image id or revision. 
	uint32_t	pixel_data_offset;	// $0A-$0D  Offset to start of actual pixel data. 

	uint32_t 	hdr_size;		// $0A-$11  Size of data header, usually 40 bytes. 
	uint32_t	width;			// $12-$15  Width of bitmap in pixels. 
	uint32_t	height;			// $16-$19  Height of bitmap in pixels. 
	uint16_t	num_color_planes;	// $1A-$1B  Number of color planes. Usually 01 
	uint16_t	bits_per_pixel;		// $1C-$1D  Number of bits per pixel. Sets color mode. 
	uint32_t	compression_mode;	// $1E-$21  Non-lossy compression mode in use, 0 - None 
	uint32_t	data_size;		// $22-$25  Size of stored pixel data 
	uint32_t	width_resolution;	// $26-$29  Width resolution in pixels per meter 
	uint32_t	height_resolution;	// $2A-$2D  Height resolution in pixels per meter 
	uint32_t	colors_used;		// $2E-$31  Number of colors actually used. 
	uint32_t	important_colors;	// $32-$35  Number of important colors 

	static inline unsigned align4(unsigned i)
	{
	}

public:

	inline uint32_t getWidth() const
	{
		return width;
	}

	inline uint32_t getHeight() const
	{
		return height;
	}

	inline uint16_t getBitsPerPixel() const
	{
		return bits_per_pixel;
	}

	inline uint32_t getBytesInRow() const
	{
		uint32_t v = getBitsPerPixel() * getWidth() / 8;
		return 	(v + 3) & ~3;
	}

	inline static uint32_t getBytesInRow(int bpp, int width)
	{
		uint32_t v = bpp * width / 8;
		return 	(v + 3) & ~3;
	}

	//
	// stub constructor
	//
	BmpHeader(uint32_t w, uint32_t h)
		: signature { 'B', 'M' }
		, length{ sizeof(*this) + h * getBytesInRow(24, w) } 
		, reseved1{ 0 }
		, width{ w }
		, height{ h }
		, pixel_data_offset{ sizeof(*this) }
		, hdr_size{ 40 }
		, num_color_planes{ 1 }
		, bits_per_pixel{ 24 }
		, compression_mode{ 0 }
		, data_size { h * getBytesInRow(24, w) }
		, width_resolution{ 0 }
		, height_resolution{ 0 }
		, colors_used{ 0 }
		, important_colors{ 0 }
	{
	}
};
#pragma pack(pop)



BmpLogger::BmpLogger(const std::string& logFolder)
	: _logFolder{ logFolder }
	, _pixels{ 4 }
	, _vpWidth{ 1 }
	, _vpHeight{ 1 }
{
	::CreateDirectoryA(logFolder.c_str(), nullptr);
}


BmpLogger::~BmpLogger()
{
}

void BmpLogger::onViewportResize(int width, int height)
{
	_vpWidth = width;
	_vpHeight = height;
	_pixels.resize(width * height * 4);
}

void BmpLogger::onNewFrame()
{
	// Capture the actual pixels 
	glReadPixels(0, 0, _vpWidth, _vpHeight, GL_RGBA, GL_UNSIGNED_BYTE, &_pixels[0]);

	// todo: format the name with leading zeroes
	std::ostringstream str;
	str  << _logFolder << "\\" << std::setw(8) << std::setfill('0') << _nextSeq++ << ".bmp";
	std::string name = str.str();

	BmpHeader hdr{ static_cast<uint32_t>(_vpWidth), static_cast<uint32_t>(_vpHeight) };

	FILE* f = fopen(name.c_str(), "wb");
	if (f != nullptr)
	{
		std::vector<unsigned char> rowData(hdr.getBytesInRow());

		if (fwrite(&hdr, sizeof(hdr), 1, f) == 1)
		{
			// BMP is a weird one, stored in a reverse order
			for (int row = 0; row < _vpHeight; ++row)
			{
				unsigned char* row_data = &_pixels[row * _vpWidth * 4]; // src img is RGBA
				for (int pxIdx = 0; pxIdx < _vpWidth; ++pxIdx)
				{
					rowData[3 * pxIdx + 0] = row_data[4 * pxIdx + 2];
					rowData[3 * pxIdx + 1] = row_data[4 * pxIdx + 1];
					rowData[3 * pxIdx + 2] = row_data[4 * pxIdx + 0];
				}

				fwrite(&rowData[0], rowData.size(), 1, f);
			}
		}

		fclose(f);
	}
}
