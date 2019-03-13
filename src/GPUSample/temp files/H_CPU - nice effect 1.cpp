#include "H_CPU.h"
#include <cmath>


void HSVtoRGB(float *r, float *g, float *b, float h, float s, float v)
{
	int i;
	float f, p, q, t;
	if (s == 0) {
		*r = *g = *b = v;
		return;
	}
	h /= 60; // sector 0 to 5
	i = floor(h);
	f = h - i; // factorial part of h
	p = v * (1 - s);
	q = v * (1 - s * f);
	t = v * (1 - s * (1 - f));
	switch (i) {
	case 0:
		*r = v;
		*g = t;
		*b = p;
		break;
	case 1:
		*r = q;
		*g = v;
		*b = p;
		break;
	case 2:
		*r = p;
		*g = v;
		*b = t;
		break;
	case 3:
		*r = p;
		*g = q;
		*b = v;
		break;
	case 4:
		*r = t;
		*g = p;
		*b = v;
		break;
	default: // case 5:
		*r = v;
		*g = p;
		*b = q;
		break;
	}
}

Image* Filters::H_CPU::Filter(Image* sourceImageOne) {


	// Create a new image that reserves space for the destination image
	Image* destinationImage = new Image(sourceImageOne->GetWidth(), sourceImageOne->GetHeight());

	unsigned int imgWidth = sourceImageOne->GetWidth();
	unsigned int imgHeight = sourceImageOne->GetHeight();
	unsigned int pixels = imgWidth * imgHeight;
	unsigned char* pixelDataSource = sourceImageOne->GetPixelData();
	unsigned char* pixelDataDestination = destinationImage->GetPixelData();
	
	std::vector<unsigned int> values;
	unsigned int frequencies[256] = { 0 };
	unsigned int CDF[256];
	float CDF_Norm[256];

	// CPU implementation of equalization
	for (unsigned int x = 0; x < imgWidth; x++)
	{
		for (unsigned int y = 0; y < imgHeight; y++)
		{
			unsigned int red = pixelDataSource[(y * imgWidth + x) * 4 + 0];
			unsigned int green = pixelDataSource[(y * imgWidth + x) * 4 + 1];
			unsigned int blue = pixelDataSource[(y * imgWidth + x) * 4 + 2];

			unsigned int value = 0;

			value = red > green ? (red > blue ? red : blue) : (green > blue ? green : blue);

			values.push_back(value);
		}
	}

	//pmf
	for (unsigned int i = 0; i < values.size(); i++) {
		int index = (int)values.at(i);
		frequencies[index] = frequencies[index] + 1;
	}

	//cdf
	unsigned int CDF_min = 9999999999;
	for (unsigned int i = 0; i < 256; i++) {
		if (i == 0) { CDF[i] = frequencies[i]; }
		else {
			CDF[i] = frequencies[i] + CDF[i - 1];
		}
		if (CDF[i] < CDF_min && CDF[i] > 0) CDF_min = CDF[i];
	}

	//cdf-norm
	for (unsigned int i = 0; i < 256; i++) {
		double t = (CDF[i] - CDF_min);
		double t2 = imgHeight * imgWidth;
		double t3 = (double)((t / (t2 - CDF_min)) * 255);
		CDF_Norm[i] = t3;
		//std::cout << CDF_Norm[i] << "\n";
	}

	for (unsigned int x = 0; x < imgWidth; x++)
	{
		for (unsigned int y = 0; y < imgHeight; y++)
		{
			unsigned int red = pixelDataSource[(y * imgWidth + x) * 4 + 0];
			unsigned int green = pixelDataSource[(y * imgWidth + x) * 4 + 1];
			unsigned int blue = pixelDataSource[(y * imgWidth + x) * 4 + 2];

			int hue;
			int saturation;
			 int value = 0;
			value = red > green ? (red > blue ? red : blue) : (green > blue ? green : blue);
			value = CDF_Norm[value];
			if (value == 0) {
				hue = 0;
				saturation = 0;
			}
			else {
				saturation = 255 * long(255 - 0) / value;
				if (saturation == 0) { hue = 0; }
				else {
					if (255 == red) hue = 0 + 43 * (green - blue) / (255 - 0);
					else if (255 == green) hue = 85 + 43 * (blue - red) / (255 - 0);
					else hue = 171 + 43 * (red - green) / (255 - 0);
				}
			}

			float r; float g; float b;

			HSVtoRGB(&r, &g , &b, hue, saturation, value);
			std::cout << r << "\n";
			pixelDataDestination[(y * imgWidth + x) * 4 + 0] = (unsigned char)r;
			pixelDataDestination[(y * imgWidth + x) * 4 + 1] = (unsigned char)g;
			pixelDataDestination[(y * imgWidth + x) * 4 + 2] = (unsigned char)b;
			pixelDataDestination[(y * imgWidth + x) * 4 + 3] = pixelDataSource[(y * imgWidth + x) * 4 + 3];
		}
	}

	return destinationImage;
}

void RGBtoHSV(float r, float g, float b, float *h, float *s, float *v) {
	float min, max, delta;
	min = min(r, g, b);
	max = max(r, g, b);
	*v = max; // v
	delta = max - min;
	if (max != 0) *s = delta / max; // s
	else {
		// r = g = b = 0 // s = 0, v is undefined
		*s = 0;
		*h = -1;
		return;
	}
	if (r == max) *h = (g - b) / delta;		// between yellow & magenta
	else if (g == max) *h = 2 + (b - r) / delta;	// between cyan & yellow
	else *h = 4 + (r - g) / delta;	// between magenta & cyan
	*h *= 60; // degrees
	if (*h < 0) *h += 360;
}

