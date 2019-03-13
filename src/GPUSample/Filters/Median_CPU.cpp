#include "Median_CPU.h"
#include <cmath>
#include <list>

Image* Filters::Median_CPU::Filter(Image* sourceImageOne) {
	// Create a new image that reserves space for the destination image
	Image* destinationImage = new Image(sourceImageOne->GetWidth(), sourceImageOne->GetHeight());

	unsigned int imgWidth = sourceImageOne->GetWidth();
	unsigned int imgHeight = sourceImageOne->GetHeight();
	unsigned char* pixelDataSource = sourceImageOne->GetPixelData();
	unsigned char* pixelDataDestination = destinationImage->GetPixelData();

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
				std::list<int> mylist;
				mylist.push_back(pixelDataSource[(y_Left * imgWidth + x_Left) * 4 + i]);
				mylist.push_back(pixelDataSource[(y_Left * imgWidth + x) * 4 + i]);
				mylist.push_back(pixelDataSource[(y_Left * imgWidth + x_Right) * 4 + i]);
				mylist.push_back(pixelDataSource[(y * imgWidth + x_Left) * 4 + i]);
				mylist.push_back(pixelDataSource[(y * imgWidth + x) * 4 + i]);
				mylist.push_back(pixelDataSource[(y * imgWidth + x_Right) * 4 + i]);
				mylist.push_back(pixelDataSource[(y_Right * imgWidth + x_Left) * 4 + i]);
				mylist.push_back(pixelDataSource[(y_Right * imgWidth + x) * 4 + i]);
				mylist.push_back(pixelDataSource[(y_Right * imgWidth + x_Right) * 4 + i]);
				mylist.sort();
				auto itr = mylist.begin();
				for (int i = 0; i < mylist.size() / 2; i++) {
					itr++;
				}
				pixelDataDestination[(y * imgWidth + x) * 4 + i] = *itr;
			}
			pixelDataDestination[(y * imgWidth + x) * 4 + 3] = pixelDataSource[(y * imgWidth + x) * 4 + 3];
		}
	}
	return destinationImage;
}