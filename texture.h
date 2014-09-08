/*
* "PS2" Application Framework
*
* University of Abertay Dundee
* May be used for educational purposed only
*
* Author - Dr Henry S Fortuna
*
* $Revision: 1.2 $
* $Date: 2007/08/19 12:45:13 $
*
*/

#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <sps2lib.h>
#include <sps2tags.h>
#include "PS2Defines.h"

// All the following is the layout of a bitmap file header (the bit before
// the actual image data). The __attribute__((packed)) tells the compiler
// not to insert any annoying padding. (If it was padded we wouldn't be
// able to read it in in one go easily)
#define BITMAP_ID      0x4D42

struct BitmapFileHeader
{
	uint16  bfType;
    uint32  bfSize;
    uint16  bfReserved1;
    uint16  bfReserved2;
    uint32  bfOffBits;
} __attribute__((packed));

/*
The BITMAPFILEHEADER structure contains information about the type, size, and
layout of a device-independent bitmap (DIB) file.

Member          Description

bfType          Specifies the type of file. This member must be BM. (0x4D42)
bfSize          Specifies the size of the file, in bytes. 
bfReserved1     Reserved; must be set to zero. 
bfReserved2     Reserved; must be set to zero.
bfOffBits       Specifies the byte offset from the BITMAPFILEHEADER structure
				to the actual bitmap data in the file.
*/


struct BitmapInfoHeader
{
	uint32	biSize;
	int32	biWidth;
	int32	biHeight;
	uint16	biPlanes;
	uint16	biBitCount;
	uint32	biCompression;
	uint32	biSizeImage;
	int32	biXPelsPerMeter;
	int32	biYPelsPerMeter;
	uint32	biClrUsed;
	uint32	biClrImportant;
} __attribute__((packed));

/*
The BITMAPINFOHEADER structure contains information about the dimensions and
color format of a Windows 3.0 or later device-independent bitmap (DIB).

Member          Description

biSize          number of bytes required by the BITMAPINFOHEADER structure.

biWidth         Specifies the width of the bitmap, in pixels. 
biHeight		Specifies the height of the bitmap, in pixels. 

biPlanes		Specifies the number of planes for the target device. This
				member must be set to 1.

biBitCount      Specifies the number of bits per pixel. This value must be 1,
				4, 8, or 24.

biCompression   Specifies the type of compression for a compressed bitmap. It
				can be one of the following values:

Value           Meaning

BI_RGB          Specifies that the bitmap is not compressed. 

BI_RLE8         Specifies a run-length encoded format for bitmaps with 8 bits
				per pixel. The compression format is a 2-byte format consisting of a count
				byte followed by a byte containing a color index.  For more information,
				see the following Comments section.

BI_RLE4         Specifies a run-length encoded format for bitmaps with 4 bits
				per pixel. The compression format is a 2-byte format consisting of a count
				byte followed by two word-length color indexes.  For more information, see
				the following Comments section.

biSizeImage     Specifies the size, in bytes, of the image. It is valid to
				set this member to zero if the bitmap is in the BI_RGB format.

biXPelsPerMeter Specifies the horizontal resolution, in pixels per meter, of
				the target device for the bitmap. An application can use this 
				value to select	a bitmap from a resource group that best matches 
				the characteristics of the current device.

biYPelsPerMeter Specifies the vertical resolution, in pixels per meter, of
				the target device for the bitmap.

biClrUsed       Specifies the number of color indexes in the color table
				actually used by the bitmap. If this value is zero, the bitmap uses the
				maximum number of colors corresponding to the value of the biBitCount
				member.	For more information on the maximum sizes of the color table, 
				see the	description of the BITMAPINFO structure earlier in this topic.

				If the biClrUsed member is nonzero, it specifies the actual number of 
				colors that the graphics engine or device driver will access if the
				biBitCountmember is less than 24. If biBitCount is set to 24, 
				biClrUsed specifies the	size of the reference color table used
				to optimize performance of Windows color palettes.  
				If the bitmap is a packed bitmap (that is, a bitmap in which
				the bitmap array immediately follows the BITMAPINFO header and which is
				referenced by a single pointer), the biClrUsed member must be set to zero or
				to the actual size of the color table.

biClrImportant  Specifies the number of color indexes that are considered
				important for displaying the bitmap. If this value is zero, all colors are
				important.
*/

struct BitmapFile
{
	BitmapFileHeader  bitmapfileheader;
	BitmapInfoHeader  bitmapinfoheader;
} __attribute__((packed));

class CTexture
{
public:
	CTexture();
	~CTexture();

	bool LoadBitmap(const char * strFilename, bool bBlackTransparent = false, bool bRedAsAlpha = false);
	uint64 Upload(uint32 uGSPage);	// This function returns tex0, incase you want to modify it.
	void Select(uint64 Tex0 = 0);	// If you modified tex0, you can pass it back here. Otherwise the
									// function will use its own values for tex0.

private:
	int m_iTexAddr;
	int m_iClutAddr;

	uint64 m_iTex0;

	int m_iDataSize;

	unsigned char m_pOriginalPalette[256*4];
	unsigned int m_pPalette32[256];

	int m_iImageDepth;
	int m_iPalBytesPerPixel;

	int m_iWidth, m_iHeight;
	int m_l2Width, m_l2Height;

	bool m_bValid;
};

#endif