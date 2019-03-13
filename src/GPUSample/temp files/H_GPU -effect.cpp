#include "H_GPU.h"
#include <CL/cl.h>
#include <iostream>
#include <fstream>

Image* Filters::H_GPU::Filter(Image* sourceImageOne) {
	Image* destinationImage = new Image(sourceImageOne->GetWidth(), sourceImageOne->GetHeight());

	unsigned int imgWidth = sourceImageOne->GetWidth();
	unsigned int imgHeight = sourceImageOne->GetHeight();
	unsigned char* pixelDataSource = sourceImageOne->GetPixelData();
	unsigned char* pixelDataDestination = destinationImage->GetPixelData();
	const size_t pixelsInBytes = imgWidth * imgHeight * 4 * sizeof(char);
	const cl_int Size = 256;
	unsigned int Histogram[Size];
	for (int index = 0; index < Size; ++index) { Histogram[index] = 0; }


	std::ifstream inFile;
	int c = inFile.peek();
	std::string output;
	inFile.open("Hist.cl");
	if (!inFile) {
		std::cout << "Unable to open file";
		exit(1); // terminate with error
	}
	while (!(inFile.eof())) {
		std::string temp;
		inFile >> temp;
		output = output + temp + " \n";

	}
	inFile.close();
	std::string kernelProgram(output);


	cl::Program kernels;
	cl::CommandQueue clQueue = (cl::CommandQueue)m_Context;

	if (m_Context.BuildKernels(kernelProgram, kernels)) {
		cl_int result = CL_SUCCESS;

		// define work-items and workgroup 
		cl::NDRange rangeOffset(0, 0);
		cl::NDRange rangeGlobal(imgWidth, imgHeight);
		cl::NDRange rangeLocal(8, 4, 0);

		// create buffers and upload all buffers
		//cl::Buffer bufHist(begin(Histogram), end(Histogram), true);
		cl::Buffer bufferIn((cl::Context)m_Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, cl::size_type(pixelsInBytes), pixelDataSource);			// no upload!
		cl::Buffer bufHist((cl::Context)m_Context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, cl::size_type(Size * sizeof(cl_int)), &Histogram);		// no upload required (will store the result later)
		cl::Buffer bufferOut((cl::Context)m_Context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, cl::size_type(pixelsInBytes), pixelDataDestination); // no upload required (will store the result later)

		char *mappedPointerIn = (char*)clQueue.enqueueMapBuffer(bufferIn, CL_TRUE, CL_MAP_WRITE, 0, pixelsInBytes, 0, 0, &result);	// expicit upload
		clQueue.finish();

		if (result == CL_SUCCESS) {
			memcpy(mappedPointerIn, pixelDataSource, pixelsInBytes);
			cl::Event endEvent;
			result = clQueue.enqueueUnmapMemObject(bufferIn, mappedPointerIn, 0, &endEvent);
			if (result == CL_SUCCESS) { endEvent.wait(); }
		}
		else {
			std::cout << "Map input buffer:" << ClErrorToString(result) << std::endl;
		}

		//char *mappedPointerIn = (char*)clQueue.enqueueMapBuffer(bufferIn, CL_TRUE, CL_MAP_WRITE, 0, pixelsInBytes, 0, 0, &result);	// explicit upload
		//clQueue.finish();
		//if (result == CL_SUCCESS) {
		//	//memcpy(mappedPointerIn, &bufHist, Size);
		//	memcpy(mappedPointerIn, pixelDataSource, pixelsInBytes);
		//	cl::Event endEvent;
		//	//result = clQueue.enqueueUnmapMemObject(bufHist, mappedPointerIn, 0, &endEvent);
		//	result = clQueue.enqueueUnmapMemObject(bufferIn, mappedPointerIn, 0, &endEvent);
		//	if (result == CL_SUCCESS) { endEvent.wait(); }
		//}
		//else {
		//	std::cout << "Map input buffer:" << ClErrorToString(result) << std::endl;
		//}

		// define functor
		//auto Hist = cl::KernelFunctor< cl::Buffer&, cl::Buffer&, cl::Buffer&>(kernels, "Hist");
		auto Hist = cl::KernelFunctor< cl::Buffer&, cl::Buffer&>(kernels, "Hist");
		// execute functor ( == kernel Hist) with input arguments and output argument 
		//cl::Event event = Hist(cl::EnqueueArgs(rangeGlobal, rangeLocal), bufferIn, bufHist, bt, result);
		cl::Event event = Hist(cl::EnqueueArgs(rangeGlobal, rangeLocal), bufferIn, bufferOut, result);

		// if success, download results to host
		//error
		if (result == CL_SUCCESS) {
			// copy back the result buffer form the device to the host image buffer			
			char *mappedPointer = (char*)clQueue.enqueueMapBuffer(bufferOut, CL_TRUE, CL_MAP_READ, 0, pixelsInBytes, 0, 0, &result);
			if (result == CL_SUCCESS) {
				memcpy(pixelDataDestination, mappedPointer, pixelsInBytes);
				cl::Event endEvent;
				result = clQueue.enqueueUnmapMemObject(bufferOut, mappedPointer, 0, &endEvent);
				if (result == CL_SUCCESS) { endEvent.wait(); }
			}
			//// copy back the result buffer from the device to the host image buffer			
			//char *mappedPointer = (char*)clQueue.enqueueMapBuffer(bufHist, CL_TRUE, CL_MAP_READ, 0, Size, 0, 0, &result);
			//if (result == CL_SUCCESS) {
			//	memcpy(Histogram, mappedPointer, Size);
			//	cl::Event endEvent;
			//	result = clQueue.enqueueUnmapMemObject(bufHist, mappedPointer, 0, &endEvent);
			//	if (result == CL_SUCCESS)
			//	{
			//		endEvent.wait();
			//	}
			//}

		}
		else {
			//std::cout << mappedPointer << std::endl;

			std::cout << "Problem invoking kernel Hist;" << std::endl;
			std::cout << " error " << ClErrorToString(result) << std::endl;
			std::cout << " error code: " << (int)result << std::endl;
		}
	}
	else {
		std::cout << "Problem building kernel Hist;" << std::endl;
		std::cout << " build info:" << m_Context.GetBuildOutput() << std::endl;
	}

	for (size_t i = 0; i < 255; i++)
	{
		std::cout << (int)pixelDataDestination[i] << " ";
		std::cout << std::endl;
	}
	return destinationImage;
}