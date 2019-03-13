#include "ED_CPU.h"
#include <cmath>

Image* Filters::ED_CPU::Filter(Image* sourceImageOne)
{
	// Create a new image that reserves space for the destination image
	Image* destinationImage = new Image(sourceImageOne->GetWidth(), sourceImageOne->GetHeight());

	unsigned int imgWidth = sourceImageOne->GetWidth();
	unsigned int imgHeight = sourceImageOne->GetHeight();
	unsigned char* pixelDataSource = sourceImageOne->GetPixelData();
	unsigned char* pixelDataDestination = destinationImage->GetPixelData();
	
	int CF[9] = { -1,0,1,-2,0,2,-1,0,1 };
	int CF2[9] = { 1,2,1,0,0,0,-1,-2,-1 };
	for (int x = 0; x < imgWidth; x++) {
		int x_Left = x - 1;
		if (x_Left < 0) { x_Left = x; }
		int x_Right = x + 1;
		if (x_Right == imgWidth) { x_Right = x; }
		for (int y = 0; y < imgHeight; y++) {
			int y_Left = y - 1;
			if (y_Left < 0) { y_Left = y; }
			int y_Right = y + 1;
			if (y_Right == imgHeight) { y_Right = y; }
			for (int i = 0; i < 3; i++) {
				int gx = (int)((
					pixelDataSource[(y_Left * imgWidth + x_Left) * 4 + i] * CF[0] +
					pixelDataSource[(y_Left * imgWidth + x) * 4 + i] * CF[1] +
					pixelDataSource[(y_Left * imgWidth + x_Right) * 4 + i] * CF[2] +
					pixelDataSource[(y * imgWidth + x_Left) * 4 + i] * CF[3] +
					pixelDataSource[(y * imgWidth + x) * 4 + i] * CF[4] +
					pixelDataSource[(y * imgWidth + x_Right) * 4 + i] * CF[5] +
					pixelDataSource[(y_Right * imgWidth + x_Left) * 4 + i] * CF[6] +
					pixelDataSource[(y_Right * imgWidth + x) * 4 + i] * CF[7] +
					pixelDataSource[(y_Right * imgWidth + x_Right) * 4 + i] * CF[8]) / 1.0);

				int gy = (int)((
					pixelDataSource[(y_Left * imgWidth + x_Left) * 4 + i] * CF2[0] +
					pixelDataSource[(y_Left * imgWidth + x) * 4 + i] * CF2[1] +
					pixelDataSource[(y_Left * imgWidth + x_Right) * 4 + i] * CF2[2] +
					pixelDataSource[(y * imgWidth + x_Left) * 4 + i] * CF2[3] +
					pixelDataSource[(y * imgWidth + x) * 4 + i] * CF2[4] +
					pixelDataSource[(y * imgWidth + x_Right) * 4 + i] * CF2[5] +
					pixelDataSource[(y_Right * imgWidth + x_Left) * 4 + i] * CF2[6] +
					pixelDataSource[(y_Right * imgWidth + x) * 4 + i] * CF2[7] +
					pixelDataSource[(y_Right * imgWidth + x_Right) * 4 + i] * CF2[8]) / 1.0);
				//pixelDataDestination[(y * imgWidth + x) * 4 + i] = abs(gx) + abs(gy);// faster than with sqrt and pow
				pixelDataDestination[(y * imgWidth + x) * 4 + i] = sqrt(pow(gx, 2) + pow(gy, 2));
			}
			pixelDataDestination[(y * imgWidth + x) * 4 + 3] = pixelDataSource[(y * imgWidth + x) * 4 + 3];
		}
	}
	return destinationImage;
}