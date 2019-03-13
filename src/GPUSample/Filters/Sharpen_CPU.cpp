#include "Sharpen_CPU.h"

Image* Filters::Sharpen_CPU::Filter(Image* sourceImageOne)
{
	// Create a new image that reserves space for the destination image
	Image* destinationImage = new Image(sourceImageOne->GetWidth(), sourceImageOne->GetHeight());

	unsigned int imgWidth = sourceImageOne->GetWidth();
	unsigned int imgHeight = sourceImageOne->GetHeight();
	unsigned char* pixelDataSource = sourceImageOne->GetPixelData();
	unsigned char* pixelDataDestination = destinationImage->GetPixelData();

	signed int filter[10] = { 1 ,0 ,-1 ,0 ,-1 ,5 ,-1 ,0 ,-1 ,0 };


	// CPU implementation of Sharpen
	for (unsigned int x = 0; x < imgWidth; x++)
	{
		for (unsigned int y = 0; y < imgHeight; y++)
		{
			//special cases
			//corners
			//left up corner
			if (x < 1 && y < 1) {

				for (int i = 0; i < 3; i++) {
					pixelDataDestination[(y * imgWidth + x) * 4 + i] =
						(int)
						(filter[1] * pixelDataSource[((y)* imgWidth + (x)) * 4 + i] + filter[2] * pixelDataSource[((y)* imgWidth + (x)) * 4 + i] + filter[3] * pixelDataSource[((y)* imgWidth + (x + 1)) * 4 + i]
							+ filter[4] * pixelDataSource[((y)* imgWidth + (x)) * 4 + i] + filter[5] * pixelDataSource[((y)* imgWidth + (x)) * 4 + i] + filter[6] * pixelDataSource[((y)* imgWidth + (x + 1)) * 4 + i]
							+ filter[7] * pixelDataSource[((y + 1) * imgWidth + (x)) * 4 + i] + filter[8] * pixelDataSource[((y + 1) * imgWidth + (x)) * 4 + i] + filter[9] * pixelDataSource[((y + 1) * imgWidth + (x + 1)) * 4 + i]) / filter[0];
				}

			}
			//left down corner
			else if (x < 1 && y >= (imgHeight - 1)) {

				for (int i = 0; i < 3; i++) {
					pixelDataDestination[(y * imgWidth + x) * 4 + i] =
						(int)
						(filter[1] * pixelDataSource[((y - 1) * imgWidth + (x)) * 4 + i] + filter[2] * pixelDataSource[((y - 1) * imgWidth + (x)) * 4 + i] + filter[3] * pixelDataSource[((y - 1) * imgWidth + (x + 1)) * 4 + i]
							+ filter[4] * pixelDataSource[((y)* imgWidth + (x)) * 4 + i] + filter[5] * pixelDataSource[((y)* imgWidth + (x)) * 4 + i] + filter[6] * pixelDataSource[((y)* imgWidth + (x + 1)) * 4 + i]
							+ filter[7] * pixelDataSource[((y)* imgWidth + (x)) * 4 + i] + filter[8] * pixelDataSource[((y)* imgWidth + (x)) * 4 + i] + filter[9] * pixelDataSource[((y)* imgWidth + (x + 1)) * 4 + i]) / filter[0];
				}


			}
			//right down corner
			else if (y < 1 && x >= (imgWidth - 1)) {

				for (int i = 0; i < 3; i++) {
					pixelDataDestination[(y * imgWidth + x) * 4 + i] =
						(int)
						(filter[1] * pixelDataSource[((y)* imgWidth + (x - 1)) * 4 + i] + filter[2] * pixelDataSource[((y)* imgWidth + (x)) * 4 + i] + filter[3] * pixelDataSource[((y)* imgWidth + (x)) * 4 + i]
							+ filter[4] * pixelDataSource[((y)* imgWidth + (x - 1)) * 4 + i] + filter[5] * pixelDataSource[((y)* imgWidth + (x)) * 4 + i] + filter[6] * pixelDataSource[((y)* imgWidth + (x)) * 4 + i]
							+ filter[7] * pixelDataSource[((y + 1) * imgWidth + (x - 1)) * 4 + i] + filter[8] * pixelDataSource[((y + 1) * imgWidth + (x)) * 4 + i] + filter[9] * pixelDataSource[((y + 1) * imgWidth + (x)) * 4 + i]) / filter[0];
				}


			}
			//right down corner
			else if (x >= (imgWidth - 1) && y >= (imgHeight - 1)) {

				for (int i = 0; i < 3; i++) {
					pixelDataDestination[(y * imgWidth + x) * 4 + i] =
						(int)
						(filter[1] * pixelDataSource[((y - 1) * imgWidth + (x - 1)) * 4 + i] + filter[2] * pixelDataSource[((y - 1) * imgWidth + (x)) * 4 + i] + filter[3] * pixelDataSource[((y - 1) * imgWidth + (x)) * 4 + i]
							+ filter[4] * pixelDataSource[((y)* imgWidth + (x - 1)) * 4 + i] + filter[5] * pixelDataSource[((y)* imgWidth + (x)) * 4 + i] + filter[6] * pixelDataSource[((y)* imgWidth + (x)) * 4 + i]
							+ filter[7] * pixelDataSource[((y)* imgWidth + (x - 1)) * 4 + i] + filter[8] * pixelDataSource[((y)* imgWidth + (x)) * 4 + i] + filter[9] * pixelDataSource[((y)* imgWidth + (x)) * 4 + i]) / filter[0];
				}

			}
			//sides
			//left side
			else if (x<1) {
				for (int i = 0; i < 3; i++) {
					pixelDataDestination[(y * imgWidth + x) * 4 + i] =
						(int)
						(filter[1] * pixelDataSource[((y - 1) * imgWidth + (x)) * 4 + i] + filter[2] * pixelDataSource[((y - 1) * imgWidth + (x)) * 4 + i] + filter[3] * pixelDataSource[((y - 1) * imgWidth + (x + 1)) * 4 + i]
							+ filter[4] * pixelDataSource[((y)* imgWidth + (x)) * 4 + i] + filter[5] * pixelDataSource[((y)* imgWidth + (x)) * 4 + i] + filter[6] * pixelDataSource[((y)* imgWidth + (x + 1)) * 4 + i]
							+ filter[7] * pixelDataSource[((y + 1) * imgWidth + (x)) * 4 + i] + filter[8] * pixelDataSource[((y + 1) * imgWidth + (x)) * 4 + i] + filter[9] * pixelDataSource[((y + 1) * imgWidth + (x + 1)) * 4 + i]) / filter[0];
				}
			}
			//up side
			else if (y < 1) {
				for (int i = 0; i < 3; i++) {
					pixelDataDestination[(y * imgWidth + x) * 4 + i] =
						(int)
						(filter[1] * pixelDataSource[((y)* imgWidth + (x - 1)) * 4 + i] + filter[2] * pixelDataSource[((y)* imgWidth + (x)) * 4 + i] + filter[3] * pixelDataSource[((y)* imgWidth + (x + 1)) * 4 + i]
							+ filter[4] * pixelDataSource[((y)* imgWidth + (x - 1)) * 4 + i] + filter[8] * pixelDataSource[((y)* imgWidth + (x)) * 4 + i] + filter[6] * pixelDataSource[((y)* imgWidth + (x + 1)) * 4 + i]
							+ filter[7] * pixelDataSource[((y + 1) * imgWidth + (x - 1)) * 4 + i] + filter[5] * pixelDataSource[((y + 1) * imgWidth + (x)) * 4 + i] + filter[9] * pixelDataSource[((y + 1) * imgWidth + (x + 1)) * 4 + i]) / filter[0];
				}
			}
			//right side
			else if (x >= (imgWidth - 1)) {
				for (int i = 0; i < 3; i++) {

					pixelDataDestination[(y * imgWidth + x) * 4 + i] =
						(int)
						(filter[1] * pixelDataSource[((y - 1) * imgWidth + (x - 1)) * 4 + i] + filter[2] * pixelDataSource[((y - 1) * imgWidth + (x)) * 4 + i] + filter[3] * pixelDataSource[((y - 1) * imgWidth + (x)) * 4 + i]
							+ filter[4] * pixelDataSource[((y)* imgWidth + (x - 1)) * 4 + i] + filter[5] * pixelDataSource[((y)* imgWidth + (x)) * 4 + i] + filter[6] * pixelDataSource[((y)* imgWidth + (x)) * 4 + i]
							+ filter[7] * pixelDataSource[((y + 1) * imgWidth + (x - 1)) * 4 + i] + filter[8] * pixelDataSource[((y + 1) * imgWidth + (x)) * 4 + i] + filter[9] * pixelDataSource[((y + 1) * imgWidth + (x)) * 4 + i]) / filter[0];



				}
			}
			//down side
			else if (y >= (imgHeight - 1)) {
				for (int i = 0; i < 3; i++) {
					pixelDataDestination[(y * imgWidth + x) * 4 + i] =
						(int)
						(filter[1] * pixelDataSource[((y - 1) * imgWidth + (x - 1)) * 4 + i] + filter[2] * pixelDataSource[((y - 1) * imgWidth + (x)) * 4 + i] + filter[3] * pixelDataSource[((y - 1) * imgWidth + (x + 1)) * 4 + i]
							+ filter[4] * pixelDataSource[((y)* imgWidth + (x - 1)) * 4 + i] + filter[5] * pixelDataSource[((y)* imgWidth + (x)) * 4 + i] + filter[6] * pixelDataSource[((y)* imgWidth + (x + 1)) * 4 + i]
							+ filter[7] * pixelDataSource[((y)* imgWidth + (x - 1)) * 4 + i] + filter[8] * pixelDataSource[((y)* imgWidth + (x)) * 4 + i] + filter[9] * pixelDataSource[((y)* imgWidth + (x + 1)) * 4 + i]) / filter[0];
				}
			}
			//rest of matrix
			else {
				for (int i = 0; i < 3; i++) {
					pixelDataDestination[(y * imgWidth + x) * 4 + i] =
						(int)
						(filter[1] * pixelDataSource[((y - 1) * imgWidth + (x - 1)) * 4 + i] + filter[2] * pixelDataSource[((y - 1) * imgWidth + (x)) * 4 + i] + filter[3] * pixelDataSource[((y - 1) * imgWidth + (x + 1)) * 4 + i]
							+ filter[4] * pixelDataSource[((y)* imgWidth + (x - 1)) * 4 + i] + filter[5] * pixelDataSource[((y)* imgWidth + (x)) * 4 + i] + filter[6] * pixelDataSource[((y)* imgWidth + (x + 1)) * 4 + i]
							+ filter[7] * pixelDataSource[((y + 1) * imgWidth + (x - 1)) * 4 + i] + filter[8] * pixelDataSource[((y + 1) * imgWidth + (x)) * 4 + i] + filter[9] * pixelDataSource[((y + 1) * imgWidth + (x + 1)) * 4 + i]) / filter[0];
				}
			}
			// alpha value
			pixelDataDestination[(y * imgWidth + x) * 4 + 3] = pixelDataSource[(y* imgWidth + x) * 4 + 3];


		}
	}

	return destinationImage;
}