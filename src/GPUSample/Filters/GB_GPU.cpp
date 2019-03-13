#include "GB_GPU.h"
#include <math.h>


Image* Filters::GB_GPU::Filter(Image* sourceImageOne) {
	printf("\nGB:\n");
	float PI_ = 3.14159265359f;
	int size = 3; // Has to be 3 with this kernelprogram!(window is 3x3)
	float sigma = 0.75;

	float* ret;
	uint32_t x, y;
	double center = size / 2;
	float sum = 0;
	//allocate and create the gaussian kernel
	ret = (float*)malloc(sizeof(float) * size * size);
	printf("kernel:\n");
	for (x = 0; x < size; x++) {
		for (y = 0; y < size; y++) {
			ret[y*size + x] = exp((((x - center)*(x - center) + (y - center)*(y - center)) / (2.0f*sigma*sigma))*-1.0f) / (2.0f*PI_*sigma*sigma);
			printf("%f ", ret[y*size + x]);
			sum += ret[y*size + x];
		}
		printf("\n");
	}
	printf("\nsum: %f \n\n", sum);
	printf("normalized kernel:\n");
	for (x = 0; x < size*size; x++) { ret[x] = ret[x] / sum; }
	//print the kernel so the user can see it
	for (x = 0; x < size; x++) {
		for (y = 0; y < size; y++) { printf("%f ", ret[y*size + x]); }
		printf("\n");
	}
	printf("\n\n");
	//std::cin.get();

	std::vector<float> CF(size*size, 0);
	for (int index = 0; index < size*size; ++index) { CF[index] = ret[index]; }

	unsigned int imgWidth = sourceImageOne->GetWidth();
	unsigned int imgHeight = sourceImageOne->GetHeight();
	Image* destinationImage = new Image(imgWidth, imgHeight);
	unsigned char* pixelDataSource = sourceImageOne->GetPixelData();
	unsigned char* pixelDataDestination = destinationImage->GetPixelData();
	const int pixels = imgWidth * imgHeight;
	const size_t pixelsInBytes = pixels * 4 * sizeof(char);

	//kernel code is just text, thus between ""
	std::string kernelProgram(
		"       int getColor(__global int *arr, int x, int y, int w, int h) {"
		"           x = (x < 0 ? 0 : (x > w - 1 ? w - 1 : x));"
		"           y = (y < 0 ? 0 : (y > h - 1 ? h - 1 : y));"
		"           return arr[(y * w + x)];"
		"       }"
		"       __kernel void GB(__global const int* inputImage, __global int* outputImage, global const float* matrix1){ "
		"           int rowCol = (get_global_id(1) * get_global_size(0) + get_global_id(0));"
		"           int yGlobal = get_global_id(1);"
		"           int xGlobal = get_global_id(0);"
		"           int x = get_local_id(0);"
		"           int y = get_local_id(1);"
		"           int width = get_global_size(0);"
		"           int height = get_global_size(1);"
		""
		"           __local int pixelBuffer[18][4];"
		"           pixelBuffer[x + 1][y + 1] = getColor(inputImage, xGlobal, yGlobal, width, height);"
		"           if(x == 0) {"
		"               pixelBuffer[0][y + 1] = getColor(inputImage, xGlobal - 1, yGlobal, width, height);"
		"               if(y == 0) {"
		"                   pixelBuffer[0][0] = getColor(inputImage, xGlobal - 1, yGlobal - 1, width, height);"
		"               }"
		"               if(y == get_local_size(1) - 1) {"
		"                   pixelBuffer[0][get_local_size(1) + 1] = getColor(inputImage, xGlobal - 1, yGlobal + 1, width, height);"
		"               }"
		"           }"
		"           if(x == get_local_size(0) - 1) {"
		"               pixelBuffer[get_local_size(0) + 1][y + 1] = getColor(inputImage, xGlobal + 1, yGlobal, width, height);"
		"               if(y == 0) {"
		"                   pixelBuffer[get_local_size(0) + 1][0] = getColor(inputImage, xGlobal + 1, yGlobal - 1, width, height);"
		"               }"
		"               if(y == get_local_size(1) - 1) {"
		"                   pixelBuffer[get_local_size(0) + 1][get_local_size(1) + 1] = getColor(inputImage, xGlobal + 1, yGlobal + 1, width, height);"
		"               }"
		"           }"
		"           if(y == 0) {"
		"               pixelBuffer[x + 1][0] = getColor(inputImage, xGlobal, yGlobal - 1, width, height);"
		"           }"
		"           if(y == get_local_size(1) - 1) {"
		"               pixelBuffer[x + 1][get_local_size(1) + 1] = getColor(inputImage, xGlobal, yGlobal + 1, width, height);"
		"           }"
		"           barrier(CLK_LOCAL_MEM_FENCE);"
		""
		"           float xRedDestination = 0;"
		"           float xGreenDestination = 0;"
		"           float xBlueDestination = 0;"
		"           x = x + 1;"
		"           y = y + 1;"
		"           for(int i = 0; i < 3; ++i) {"
		"               for(int j = 0; j < 3; ++j) {"
		"                   xRedDestination += matrix1[j * 3 + i] * ((pixelBuffer[x + (i - 1)][y + (j - 1)] & 0x00FF0000) >> 16);"
		"                   xGreenDestination += matrix1[j * 3 + i] * ((pixelBuffer[x + (i - 1)][y + (j - 1)] & 0x0000FF00) >> 8);"
		"                   xBlueDestination += (matrix1[j * 3 + i] * ((pixelBuffer[x + (i - 1)][y + (j - 1)] & 0x000000FF) >> 0));"
		"               }"
		"           }"
		""
		"           float redRoot = fmin(fmax(0.0,xRedDestination), 255.0);"
		"           float greenRoot = fmin(fmax(0.0, xGreenDestination), 255.0);"
		"           float blueRoot = fmin(fmax(0.0, xBlueDestination), 255.0);"
		""
		"           int alpha = 0xFF << 24;"
		"           int red = (int)(redRoot) << 16;"
		"           int green = (int)(greenRoot) << 8;"
		"           int blue = (int)(blueRoot) << 0;"
		"           int color = alpha | red | green | blue;"
		"           outputImage[rowCol] = color;"
		"	    }"
	);

	cl::Program kernels;
	// queu to submit work to
	cl::CommandQueue clQueue = (cl::CommandQueue)m_Context;
	// create a program from the text source, from string kernelProgram
	// then build the program to run in compilation
	// compilation is slow, thus first time you run the program is slow and following times its faster
	if (m_Context.BuildKernels(kernelProgram, kernels))
	{
		cl_int result = CL_SUCCESS;

		// define work-items and workgroup layout
		cl::NDRange rangeOffset(0, 0);
		cl::NDRange rangeGlobal(imgWidth, imgHeight);
		cl::NDRange rangeLocal(16, 2, 0);
		cl::NDRange computeDomain(imgWidth, imgHeight);

		// create buffers and upload all buffers
		cl::Buffer bufferFilter(begin(CF), end(CF), true);
		//cl::Buffer bufferFilter((cl::Context)m_Context, CL_MEM_READ_ONLY, cKernelSize * sizeof(float));
		cl::Buffer bufferIn((cl::Context)m_Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, cl::size_type(pixelsInBytes), pixelDataSource); // no upload!
		cl::Buffer bufferOut((cl::Context)m_Context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, cl::size_type(pixelsInBytes), pixelDataDestination); // no upload required (will store the result later)

		char *mappedPointerIn = (char*)clQueue.enqueueMapBuffer(bufferIn, CL_TRUE, CL_MAP_WRITE, 0, pixelsInBytes, 0, 0, &result);	// expicit upload
		clQueue.finish();

		if (result == CL_SUCCESS) {
			memcpy(mappedPointerIn, pixelDataSource, pixelsInBytes);
			cl::Event endEvent;
			result = clQueue.enqueueUnmapMemObject(bufferIn, mappedPointerIn, 0, &endEvent);
			if (result == CL_SUCCESS) { endEvent.wait(); }
		} else {
			std::cout << "Map input buffer:" << ClErrorToString(result) << std::endl;
		}

		// define functor
		auto GB = cl::KernelFunctor< cl::Buffer&, cl::Buffer&, cl::Buffer&>(kernels, "GB");
		
		// execute functor ( == kernel GB) with input arguments and output argument 
		cl::Event event = GB(cl::EnqueueArgs(rangeGlobal, rangeLocal), bufferIn, bufferOut, bufferFilter, result);

		// if success, download results to host
		if (result == CL_SUCCESS) {
			// copy back the result buffer form the device to the host image buffer			
			char *mappedPointer = (char*)clQueue.enqueueMapBuffer(bufferOut, CL_TRUE, CL_MAP_READ, 0, pixelsInBytes, 0, 0, &result);
			if (result == CL_SUCCESS) {
				memcpy(pixelDataDestination, mappedPointer, pixelsInBytes);
				cl::Event endEvent;
				result = clQueue.enqueueUnmapMemObject(bufferOut, mappedPointer, 0, &endEvent);
				if (result == CL_SUCCESS) { endEvent.wait(); }
			}
		} else {
			std::cout << "Problem invoking kernel GB;" << std::endl;
			std::cout << " error " << ClErrorToString(result) << std::endl;
			std::cout << " error code: " << (int)result << std::endl;
		}
	} else {
		std::cout << "Problem building kernel GB;" << std::endl;
		std::cout << " build info:" << m_Context.GetBuildOutput() << std::endl;
	}
	return destinationImage;
}