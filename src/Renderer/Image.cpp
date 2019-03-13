#include "Image.h"



/****************************************************************/
/* CONSTRUCTORS AND DESTRUCTORS                                 */
/****************************************************************/

Image::Image(unsigned char* pixelData, unsigned int width, unsigned int height)
{
	this->m_PixelData = pixelData;
	this->m_Width = width;
	this->m_Height = height;
}

Image::Image(unsigned int width, unsigned int height)
{
	this->m_PixelData = new unsigned char[4 * width * height]; // 4 characters are reserved per pixel, for the R, G, B and A components respectively
	this->m_Width = width;
	this->m_Height = height;
}

Image::~Image()
{
	if (m_PixelData)
		delete[] m_PixelData;
}


