typedef struct RgbColor
{
	int r;
	int g;
	int b;
} RgbColor;
typedef struct HsvColor
{
	int h;
	int s;
	int v;
} HsvColor;
HsvColor RgbToHsv(RgbColor rgb) {
	HsvColor hsv;
	int minn, maxx, delta;
	minn = min(rgb.r, rgb.g);
	minn = min(minn, rgb.b);
	maxx = max(rgb.r, rgb.g);
	maxx = max(maxx, rgb.b);
	hsv.v = maxx;
	delta = maxx - minn;
	if (maxx != 0)
		hsv.s = delta / maxx;
	else {
		hsv.s = 0;
		hsv.h = -1;
		return hsv;
	}
	if (rgb.r == maxx)
		hsv.h = (rgb.g - rgb.b) / delta;
	else if (rgb.g == maxx)
		hsv.h = 2 + (rgb.b - rgb.r) / delta;
	else
		hsv.h = 4 + (rgb.r - rgb.g) / delta;
	hsv.h *= 60;
	if (hsv.h < 0)
		hsv.h += 360;
	return hsv;
}
__kernel void Hist(__global const int* inputImage, __global int* histogram) {
	const size_t rowcol = (get_global_id(1) * get_global_size(0) + get_global_id(0)); 
	int color =  inputImage[rowcol];
	RgbColor rgb;
	rgb.r   = (color & 0xFF0000) >> 16 ;
	rgb.g = (color & 0x00FF00) >> 8 ;
	rgb.b  = (color & 0x0000FF) >> 0 ;
		
	HsvColor hsv;
	hsv = RgbToHsv(rgb);
	hsv.h = min((max(hsv.h, 0)), 255);
	hsv.s = min((max(hsv.s, 0)), 255);
	hsv.v = min((max(hsv.v, 0)), 255);
	atomic_add(&histogram[rowcol], hsv.v);

}