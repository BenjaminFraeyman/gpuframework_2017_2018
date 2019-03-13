#include "H_CPU.h"
#include <cmath>

typedef struct RgbColor { unsigned char r; unsigned char g; unsigned char b; } RgbColor;
typedef struct HsvColor { unsigned char h; unsigned char s; unsigned char v; } HsvColor;

RgbColor HsvToRgb(HsvColor hsv) {
	RgbColor rgb;
	unsigned char region, remainder, p, q, t;

	if (hsv.s == 0) {
		rgb.r = hsv.v;
		rgb.g = hsv.v;
		rgb.b = hsv.v;
		return rgb;
	}

	region = hsv.h / 43;
	remainder = (hsv.h - (region * 43)) * 6;
	p = (hsv.v * (255 - hsv.s)) >> 8;
	q = (hsv.v * (255 - ((hsv.s * remainder) >> 8))) >> 8;
	t = (hsv.v * (255 - ((hsv.s * (255 - remainder)) >> 8))) >> 8;

	switch (region) {
		case 0:
			rgb.r = hsv.v; rgb.g = t; rgb.b = p;
			break;
		case 1:
			rgb.r = q; rgb.g = hsv.v; rgb.b = p;
			break;
		case 2:
			rgb.r = p; rgb.g = hsv.v; rgb.b = t;
			break;
		case 3:
			rgb.r = p; rgb.g = q; rgb.b = hsv.v;
			break;
		case 4:
			rgb.r = t; rgb.g = p; rgb.b = hsv.v;
			break;
		default:
			rgb.r = hsv.v; rgb.g = p; rgb.b = q;
			break;
	}
	return rgb;
}

HsvColor RgbToHsv(RgbColor rgb) {
	HsvColor hsv;
	unsigned char rgbMin, rgbMax;
	rgbMin = rgb.r < rgb.g ? (rgb.r < rgb.b ? rgb.r : rgb.b) : (rgb.g < rgb.b ? rgb.g : rgb.b);
	rgbMax = rgb.r > rgb.g ? (rgb.r > rgb.b ? rgb.r : rgb.b) : (rgb.g > rgb.b ? rgb.g : rgb.b);

	hsv.v = rgbMax;
	if (hsv.v == 0) {
		hsv.h = 0;
		hsv.s = 0;
		return hsv;
	}

	hsv.s = 255 * long(rgbMax - rgbMin) / hsv.v;
	if (hsv.s == 0) {
		hsv.h = 0;
		return hsv;
	}

	if (rgbMax == rgb.r) hsv.h = 0 + 43 * (rgb.g - rgb.b) / (rgbMax - rgbMin);
	else if (rgbMax == rgb.g) hsv.h = 85 + 43 * (rgb.b - rgb.r) / (rgbMax - rgbMin);
	else hsv.h = 171 + 43 * (rgb.r - rgb.g) / (rgbMax - rgbMin);
	return hsv;
}

Image* Filters::H_CPU::Filter(Image* sourceImageOne) {
	// Create a new image that reserves space for the destination image
	Image* destinationImage = new Image(sourceImageOne->GetWidth(), sourceImageOne->GetHeight());

	unsigned int imgWidth = sourceImageOne->GetWidth();
	unsigned int imgHeight = sourceImageOne->GetHeight();
	unsigned int pixels = imgWidth * imgHeight;
	unsigned char* pixelDataSource = sourceImageOne->GetPixelData();
	unsigned char* pixelDataDestination = destinationImage->GetPixelData();
	const int Size = 256;

	signed int hist[Size] = { 0 };
	signed int CDF[Size] = { 0 };
	signed int CDF_Norm[Size] = { 0 };

	//pmf
	for (unsigned int x = 0; x < imgWidth; x++) {
		for (unsigned int y = 0; y < imgHeight; y++) {
			RgbColor rgb;
			rgb.r = pixelDataSource[(y * imgWidth + x) * 4 + 0];
			rgb.g = pixelDataSource[(y * imgWidth + x) * 4 + 1];
			rgb.b = pixelDataSource[(y * imgWidth + x) * 4 + 2];

			/*HsvColor hsv;
			hsv = RgbToHsv(rgb);
			hsv.h = min((max(hsv.h, 0)), 255);
			hsv.s = min((max(hsv.s, 0)), 255);
			hsv.v = min((max(hsv.v, 0)), 255);
			hist[hsv.v] = hist[hsv.v] + 1;*/

			unsigned char v = rgb.r > rgb.g ? (rgb.r > rgb.b ? rgb.r : rgb.b) : (rgb.g > rgb.b ? rgb.g : rgb.b);
			v = min((max(v, 0)), 255);
			hist[v] = hist[v] + 1;
		}
	}

	//cdf
	signed int CDF_min = 999999999; // start with max possible value
	for (unsigned int i = 0; i < Size; i++) {
		if (i == 0) { CDF[i] = hist[i]; }
		else { CDF[i] = hist[i] + CDF[i - 1]; }
		if (CDF[i] < CDF_min && CDF[i] > 0) CDF_min = CDF[i];
	}

	//cdf-norm
	for (unsigned int i = 0; i < Size; i++) { CDF_Norm[i] = min((max(round(((CDF[i] - CDF_min) * 255) / ((imgHeight * imgWidth) - CDF_min)), 0)), 255); }
		/*double t = (CDF[i] - CDF_min); double t2 = imgHeight * imgWidth; double t3 = (double)((t / (t2 - CDF_min)) * 255);
		CDF_Norm[i] = t3;
	}*/

	for (unsigned int x = 0; x < imgWidth; x++) {
		for (unsigned int y = 0; y < imgHeight; y++) {
			RgbColor rgb;
			rgb.r = pixelDataSource[(y * imgWidth + x) * 4 + 0];
			rgb.g = pixelDataSource[(y * imgWidth + x) * 4 + 1];
			rgb.b = pixelDataSource[(y * imgWidth + x) * 4 + 2];

			HsvColor hsv;
			hsv = RgbToHsv(rgb);
			hsv.h = min((max(hsv.h, 0)), 255);
			hsv.s = min((max(hsv.s, 0)), 255);
			hsv.v = min((max(hsv.v, 0)), 255);
			hsv.v = CDF_Norm[hsv.v];

			rgb = HsvToRgb(hsv);

			pixelDataDestination[(y * imgWidth + x) * 4 + 0] = rgb.r;
			pixelDataDestination[(y * imgWidth + x) * 4 + 1] = rgb.g;
			pixelDataDestination[(y * imgWidth + x) * 4 + 2] = rgb.b;
			pixelDataDestination[(y * imgWidth + x) * 4 + 3] = pixelDataSource[(y * imgWidth + x) * 4 + 3];
		}
	}
	return destinationImage;
}