#include "Sharpen_GPU.h"

Image* Filters::Sharpen_GPU::Filter(Image* sourceImageOne)
{

	Image* destinationImage = new Image(sourceImageOne->GetWidth(), sourceImageOne->GetHeight());

	unsigned int imgWidth = sourceImageOne->GetWidth();
	unsigned int imgHeight = sourceImageOne->GetHeight();
	unsigned char* pixelDataSource = sourceImageOne->GetPixelData();
	unsigned char* pixelDataDestination = destinationImage->GetPixelData();
	cl_int Size = 9;
	float filter[9] = { 0.0f ,-1.0f ,0.0f ,-1.0f ,5.0f ,-1.0f ,0.0f ,-1.0f ,0.0f };

	const int pixels = imgWidth * imgHeight;
	const size_t pixelsInBytes = pixels * 4 * sizeof(char);
	std::vector<float> A(Size, 0);

	for (int index = 0; index < Size; ++index) { A[index] = filter[index]; }

	std::string kernelProgram(
		"       int getColor(__global int *arr, int x, int y, int w, int h) {"
		"           x = (x < 0 ? 0 : (x > w - 1 ? w - 1 : x));"
		"           y = (y < 0 ? 0 : (y > h - 1 ? h - 1 : y));"
		"           return arr[(y * w + x)];"
		"       }"
		"       __kernel void Sharpen(__global const int* inputImage, __global int* outputImage, global const float* matrix1){		"
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
	cl::CommandQueue clQueue = (cl::CommandQueue)m_Context;

	if (m_Context.BuildKernels(kernelProgram, kernels))
	{
		cl_int result = CL_SUCCESS;
		// define work-items and workgroup 
		//cl::NDRange computeDomain(imgWidth, imgHeight);

		cl::NDRange rangeOffset(0, 0);
		cl::NDRange rangeGlobal(imgWidth, imgHeight);
		cl::NDRange rangeLocal(16, 2, 0);

		// create buffers and upload all buffers
		cl::Buffer bufA(begin(A), end(A), true);
		//cl::Buffer subbuffer();

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
		auto Sharpen = cl::KernelFunctor< cl::Buffer&, cl::Buffer&, cl::Buffer&>(kernels, "Sharpen");

		// execute functor ( == kernel Sharpen) with input arguments and output argument 
		cl::Event event = Sharpen(cl::EnqueueArgs(rangeGlobal, rangeLocal), bufferIn, bufferOut,  bufA, result);

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
			std::cout << "Problem invoking kernel Sharpen;" << std::endl;
			std::cout << " error " << ClErrorToString(result) << std::endl;
			std::cout << " error code: " << (int)result << std::endl;
		}
	}
	else
	{
		std::cout << "Problem building kernel Sharpen;" << std::endl;
		std::cout << " build info:" << m_Context.GetBuildOutput() << std::endl;
	}

	return destinationImage;
}