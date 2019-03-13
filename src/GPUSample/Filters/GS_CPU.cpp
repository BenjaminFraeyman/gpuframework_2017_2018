#include "GS_CPU.h"
#include <math.h> 

Image* Filters::GS_CPU::Filter(Image* sourceImageOne)
{
	Image* destinationImage = new Image(sourceImageOne->GetWidth(), sourceImageOne->GetHeight());

	unsigned int imgWidth = sourceImageOne->GetWidth();
	unsigned int imgHeight = sourceImageOne->GetHeight();
	unsigned char* pixelDataSource = sourceImageOne->GetPixelData();
	unsigned char* pixelDataDestination = destinationImage->GetPixelData();

	 int rk = 52;
	 int gk = 177;
	 int bk = 77;
	 int cmin = 30;
	 int cmax = 80;

	for (unsigned int x = 0; x < imgWidth; x++)
	{
		for (unsigned int y = 0; y < imgHeight; y++)
		{
			int dist = sqrt(
				(pow((pixelDataSource[(y * imgWidth + x) * 4 + 0] - rk), 2)) +
				(pow((pixelDataSource[(y * imgWidth + x) * 4 + 1] - gk), 2)) +
				(pow((pixelDataSource[(y * imgWidth + x) * 4 + 2] - bk), 2))
			);
			if (dist < cmin) { pixelDataDestination[(y * imgWidth + x) * 4 + 3] = 0;
			} else if (dist > cmax) { pixelDataDestination[(y * imgWidth + x) * 4 + 3] = 255;
			} else { pixelDataDestination[(y * imgWidth + x) * 4 + 3] = 255 * ((dist - cmax) / (cmax - cmin)); }

			pixelDataDestination[(y * imgWidth + x) * 4 + 0] = pixelDataSource[(y * imgWidth + x) * 4 + 0];
			pixelDataDestination[(y * imgWidth + x) * 4 + 1] = pixelDataSource[(y * imgWidth + x) * 4 + 1];
			pixelDataDestination[(y * imgWidth + x) * 4 + 2] = pixelDataSource[(y * imgWidth + x) * 4 + 2];
		}
	}
	return destinationImage;
}