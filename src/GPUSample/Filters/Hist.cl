__kernel void Hist(__global const int* inputImage,__global int* histogram){
	const size_t rowcol = (get_global_id(1) * get_global_size(0) + get_global_id(0));
	int color = inputImage[rowcol];

	int r = (color & 0xFF0000) >> 16;
	int g = (color & 0x00FF00) >> 8;
	int b = (color & 0x0000FF) >> 0;

	int v = r > g ? (r > b ? r : b) : (g > b ? g : b);
	v = min((max(v, 0)), 255);
	atomic_inc(&histogram[v]);
}