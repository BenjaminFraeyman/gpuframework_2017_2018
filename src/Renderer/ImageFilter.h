#ifndef H_IMAGEFILTER
#define H_IMAGEFILTER

#include "Image.h"

// An image filter represents a filter that can be applied to an image
class ImageFilter{

public:

	/****************************************************************/
	/* CONSTRUCTORS AND DESTRUCTORS                                 */
	/****************************************************************/

	// Default empty constructor
	ImageFilter() { }
	virtual ~ImageFilter() {}

	/****************************************************************/
	/* FILTER OPERATION                                             */
	/****************************************************************/

	// Performs a filter operation on a source image and return the resulting image
	virtual Image* Filter(Image* sourceImage) = 0;

};

#endif