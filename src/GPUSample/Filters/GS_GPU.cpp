#include "GS_GPU.h"
#include <math.h> 

Image* Filters::GS_GPU::Filter(Image* sourceImageOne)
{
	Image* destinationImage = new Image(sourceImageOne->GetWidth(), sourceImageOne->GetHeight());

	unsigned int imgWidth = sourceImageOne->GetWidth();
	unsigned int imgHeight = sourceImageOne->GetHeight();
	unsigned char* pixelDataSource = sourceImageOne->GetPixelData();
	unsigned char* pixelDataDestination = destinationImage->GetPixelData();

	cl_int randomTableSize = 71;
	const int pixels = imgWidth * imgHeight;
	const size_t pixelsInBytes = pixels * 4 * sizeof(char);
	std::vector<float> randomTable(randomTableSize, 0);

	// just build a matrix with random numbers
	for (int index = 0; index < randomTableSize; ++index) { randomTable[index] = 1.0f *  ((rand() % 1000) / 1000.0f); }
	
	std::string kernelProgram(
		"       __kernel void GS(__global const int* inputImage, __global int* outputImage, __global const float* randomTable, int randomTableSize){		"
		""
		"		const size_t rowcol = (get_global_id(1) * get_global_size(0) + get_global_id(0));"		// linear address of the pixel
		""
		"		int rk = 52;"
		"		int gk = 177;"
		"		int bk = 77;"
		"		int cmin = 30;"
		"		int cmax = 80; "
		""
		"		int color =  inputImage[rowcol];"
		"		int red   = (color & 0xFF0000) >> 16;"
		"		int green = (color & 0x00FF00) >> 8;"
		"		int blue  = (color & 0x0000FF) >> 0;"	
		""
		"		int dist = sqrt((pow((red - rk), 2)) + (pow((green - gk), 2)) + (pow((blue - bk), 2)));"
		""
		"		int alpha = 0;"
		"		if (dist < cmin) { alpha = 0;}"
		"		else if (dist > cmax) { alpha = 255;}"
		"		else { alpha = 255 * ((dist - cmax) / (cmax - cmin)); }"
		""
		"		red   = (red << 16) & 0xFF0000;"
		"		green = (green << 8)  & 0x00FF00;"
		"		blue  = (blue << 0)  & 0x0000FF;"
		""
		"		alpha = (alpha << 24) & 0xFF000000;"
		"		int newColor = alpha | red | green | blue;"
		"		outputImage[rowcol] = newColor;"
		"	}"
	);

	cl::Program kernels;
	cl::CommandQueue clQueue = (cl::CommandQueue)m_Context;

	if (m_Context.BuildKernels(kernelProgram, kernels))
	{
		cl_int result = CL_SUCCESS;

		// define work-items and workgroup layout
		cl::NDRange rangeOffset(0, 0);
		cl::NDRange rangeGlobal(imgWidth, imgHeight);
		cl::NDRange rangeLocal(16, 2, 0);

		// create buffers and upload all buffers
		cl::Buffer bufferRnd(begin(randomTable), end(randomTable), true); // implicit upload
		cl::Buffer bufferIn((cl::Context)m_Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, cl::size_type(pixelsInBytes), pixelDataSource);			// no upload!
		cl::Buffer bufferOut((cl::Context)m_Context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, cl::size_type(pixelsInBytes), pixelDataDestination);		// no upload required (will store the result later)

		char *mappedPointerIn = (char*)clQueue.enqueueMapBuffer(bufferIn, CL_TRUE, CL_MAP_WRITE, 0, pixelsInBytes, 0, 0, &result);	// expicit upload
		clQueue.finish();

		if (result == CL_SUCCESS)
		{
			memcpy(mappedPointerIn, pixelDataSource, pixelsInBytes);
			cl::Event endEvent;
			result = clQueue.enqueueUnmapMemObject(bufferIn, mappedPointerIn, 0, &endEvent);
			if (result == CL_SUCCESS)
			{
				endEvent.wait();
			}
		}
		else
		{
			std::cout << "Map input buffer:" << ClErrorToString(result) << std::endl;
		}

		// define functor
		auto GS = cl::KernelFunctor< cl::Buffer&, cl::Buffer&, cl::Buffer&, cl_int>(kernels, "GS");

		// execute functor ( == kernel GS) with input arguments and output argument 
		cl::Event event = GS(cl::EnqueueArgs(rangeGlobal, rangeLocal), bufferIn, bufferOut, bufferRnd, randomTableSize, result);

		// if success, download results to host
		if (result == CL_SUCCESS)
		{
			// copy back the result buffer form the device to the host image buffer			
			char *mappedPointer = (char*)clQueue.enqueueMapBuffer(bufferOut, CL_TRUE, CL_MAP_READ, 0, pixelsInBytes, 0, 0, &result);
			if (result == CL_SUCCESS)
			{
				memcpy(pixelDataDestination, mappedPointer, pixelsInBytes);
				cl::Event endEvent;
				result = clQueue.enqueueUnmapMemObject(bufferOut, mappedPointer, 0, &endEvent);
				if (result == CL_SUCCESS)
				{
					endEvent.wait();
				}
			}

		}
		else
		{
			std::cout << "Problem invoking kernel GS;" << std::endl;
			std::cout << " error " << ClErrorToString(result) << std::endl;
			std::cout << " error code: " << (int)result << std::endl;
		}
	}
	else
	{
		std::cout << "Problem building kernel GS;" << std::endl;
		std::cout << " build info:" << m_Context.GetBuildOutput() << std::endl;
	}

	return destinationImage;
}