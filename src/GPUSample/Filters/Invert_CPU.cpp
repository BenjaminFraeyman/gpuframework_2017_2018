#include "Invert_CPU.h"

Image* Filters::Invert_CPU::Filter(Image* sourceImageOne)
{
	Image* destinationImage = new Image(sourceImageOne->GetWidth(), sourceImageOne->GetHeight());

	unsigned int imgWidth = sourceImageOne->GetWidth();
	unsigned int imgHeight = sourceImageOne->GetHeight();
	unsigned char* pixelDataSource = sourceImageOne->GetPixelData();
	unsigned char* pixelDataDestination = destinationImage->GetPixelData();

	for (unsigned int x = 0; x < imgWidth; x++)
	{
		for (unsigned int y = 0; y < imgHeight; y++)
		{
			pixelDataDestination[(y * imgWidth + x) * 4 + 0] = 255 - pixelDataSource[(y * imgWidth + x) * 4 + 0];
			pixelDataDestination[(y * imgWidth + x) * 4 + 1] = 255 - pixelDataSource[(y * imgWidth + x) * 4 + 1];
			pixelDataDestination[(y * imgWidth + x) * 4 + 2] = 255 - pixelDataSource[(y * imgWidth + x) * 4 + 2];
			pixelDataDestination[(y * imgWidth + x) * 4 + 3] = pixelDataSource[(y * imgWidth + x) * 4 + 3];

		}
	}
	return destinationImage;
}