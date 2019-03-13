typedef struct RgbColor
{
	int r;
	int g;
	int b;
} RgbColor;
__kernel void Hist(__global const int* inputImage, __global int* histogram) {
	const size_t rowcol = (get_global_id(1) * get_global_size(0) + get_global_id(0));
	int color = inputImage[rowcol];
	RgbColor rgb;
	rgb.r = (color & 0xFF0000) >> 16;
	rgb.g = (color & 0x00FF00) >> 8;
	rgb.b = (color & 0x0000FF) >> 0;
	int v = rgb.r > rgb.g ? (rgb.r > rgb.b ? rgb.r : rgb.b) : (rgb.g > rgb.b ? rgb.g : rgb.b);
	v = (max(min(255, v), 0));
	atomic_add(&histogram[v], 1);
}