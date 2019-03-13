#ifndef H_IMAGE
#define H_IMAGE

// An image represents a bitmap image along with its metadata
class Image{

private: 
	unsigned char* m_PixelData;
	unsigned int m_Width;
	unsigned int m_Height;

public:

	/****************************************************************/
	/* CONSTRUCTORS AND DESTRUCTORS                                 */
	/****************************************************************/

	// Constructor (when pixelData is already available)
	Image(unsigned char* pixelData, unsigned int width, unsigned int height);
	// Constructor (when an empty array of pixelData needs to be reserved)
	Image(unsigned int width, unsigned int height);
	// Deconstructor
	~Image();
	
	/****************************************************************/
	/* ATTRIBUTE GETTERS AND SETTERS                                */
	/****************************************************************/
	
	// Gets a pointer to the pixel data of the image, stored in a character array. Pixel data is stored in an interleaved sequence of R (red), G (green), B (blue) and A (alpha) components. Every individual component occupies one character of the array.
	unsigned char* GetPixelData() { return m_PixelData; }
	// Gets the width of the image, i.e. the horizontal number of pixels.
	inline const unsigned int GetWidth() const { return m_Width; }
	// Gets the height of the image, i.e. the vertical number of pixels.
	inline const int GetHeight()  const { return m_Height; }

};

#endif