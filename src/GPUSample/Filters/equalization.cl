typedef struct RgbColor { int r; int g; int b; } RgbColor;
typedef struct HsvColor { int h; int s; int v; } HsvColor;

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

	hsv.s = (int)(255 * (rgbMax - rgbMin) / hsv.v);
	if (hsv.s == 0) {
		hsv.h = 0;
		return hsv;
	}

	if (rgbMax == rgb.r) hsv.h = 0 + 43 * (rgb.g - rgb.b) / (rgbMax - rgbMin);
	else if (rgbMax == rgb.g) hsv.h = 85 + 43 * (rgb.b - rgb.r) / (rgbMax - rgbMin);
	else hsv.h = 171 + 43 * (rgb.r - rgb.g) / (rgbMax - rgbMin);
	return hsv;
}

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

__kernel void equalization(__global const int* inputImage,__global int* outputImage, __global int* histE){
	const size_t rowcol = (get_global_id(1) * get_global_size(0) + get_global_id(0));
	int color =  inputImage[rowcol];

	RgbColor rgb;
	rgb.r   = (color & 0xFF0000) >> 16;
	rgb.g = (color & 0x00FF00) >> 8;
	rgb.b  = (color & 0x0000FF) >> 0;

	HsvColor hsv;
	hsv = RgbToHsv(rgb);
	hsv.h = min((max(hsv.h, 0)), 255);
	hsv.s = min((max(hsv.s, 0)), 255);
	hsv.v = min((max(hsv.v, 0)), 255);
	hsv.v = histE[hsv.v];

	RgbColor rgb2;
	rgb2 = HsvToRgb(hsv);
	int a = 0xFF << 24;
	int r = ((int)(rgb2.r <<16)) & 0xFF0000;
	int g = ((int)(rgb2.g <<8)) & 0x00FF00;
	int b = ((int)(rgb2.b <<0)) & 0x0000FF;
	int newColor = a | r | g | b;
	outputImage[rowcol] = newColor;
}