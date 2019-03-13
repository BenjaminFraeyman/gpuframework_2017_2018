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

	const cl_int Size = 256 * 4;
	int Histogram[Size];
	for (int index = 0; index < Size; ++index) { Histogram[index] = 0; }
	int CDF[Size];
	for (int index = 0; index < Size; ++index) { CDF[index] = 0; }
	int histE[Size];

	/* Kernel 1 */
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

	/* Kernel 2 */
	// usefull link: https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both?utm_medium=organic&utm_source=google_rich_qa&utm_campaign=google_rich_qa
	std::ifstream inFile2;
	c = inFile2.peek();
	std::string output2;
	inFile2.open("equalization.cl");
	if (!inFile2) {
		std::cout << "Unable to open file";
		exit(1); // terminate with error
	}
	while (!(inFile2.eof())) {
		std::string temp;
		inFile2 >> temp;
		output2 = output2 + temp + " \n";
	}
	inFile2.close();
	std::string kernelProgram2(output2);


	cl::Program kernels;
	cl::CommandQueue clQueue = (cl::CommandQueue)m_Context;
	/* First kernel --------------------------------------------------------------------------*/
	if (m_Context.BuildKernels(kernelProgram, kernels)) {
		cl_int result = CL_SUCCESS;

		// define work-items and workgroup 
		cl::NDRange rangeOffset(0, 0);
		cl::NDRange rangeGlobal(imgWidth, imgHeight);
		cl::NDRange rangeLocal(16, 8, 0);

		// create buffers and upload all buffers
		cl::Buffer bufferIn((cl::Context)m_Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, cl::size_type(pixelsInBytes), pixelDataSource);
		cl::Buffer bufHist((cl::Context)m_Context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, cl::size_type(Size * sizeof(cl_int)), &Histogram);
		char *mappedPointerIn = (char*)clQueue.enqueueMapBuffer(bufferIn, CL_TRUE, CL_MAP_WRITE, 0, pixelsInBytes, 0, 0, &result);
		clQueue.finish();

		if (result == CL_SUCCESS) {
			memcpy(mappedPointerIn, pixelDataSource, pixelsInBytes);
			cl::Event endEvent;
			result = clQueue.enqueueUnmapMemObject(bufferIn, mappedPointerIn, 0, &endEvent);
			if (result == CL_SUCCESS) { endEvent.wait(); }
		} else { std::cout << "Map input buffer:" << ClErrorToString(result) << std::endl; }

		// define functor
		auto Hist = cl::KernelFunctor< cl::Buffer&, cl::Buffer&>(kernels, "Hist");

		// execute functor ( == kernel Hist) with input arguments and output argument 
		cl::Event event = Hist(cl::EnqueueArgs(rangeGlobal, rangeLocal), bufferIn, bufHist, result);

		// if success, download results to host
		if (result == CL_SUCCESS) {
			// copy back the result buffer from the device to the host image buffer			
			char *mappedPointer = (char*)clQueue.enqueueMapBuffer(bufHist, CL_TRUE, CL_MAP_READ, 0, Size, 0, 0, &result);
			if (result == CL_SUCCESS) {
				memcpy(Histogram, mappedPointer, Size);
				cl::Event endEvent;
				result = clQueue.enqueueUnmapMemObject(bufHist, mappedPointer, 0, &endEvent);
				if (result == CL_SUCCESS) { endEvent.wait(); }
			}
		} else {
			std::cout << "Problem invoking kernel Hist;" << std::endl;
			std::cout << " error " << ClErrorToString(result) << std::endl;
			std::cout << " error code: " << (int)result << std::endl;
		}
	} else {
		std::cout << "Problem building kernel Hist;" << std::endl;
		std::cout << " build info:" << m_Context.GetBuildOutput() << std::endl;
	}

	//for (size_t i = 0; i < 256; i++) { std::cout << Histogram[i] << std::endl; }

	//CDF bepalen
	signed int CDFmin = Size;
	for (int i = 0; i < 256; i++) {
		if (i == 0) { CDF[i] = Histogram[i]; }
		else { CDF[i] = CDF[i - 1] + Histogram[i]; }
		if (CDF[i] != 0 && CDF[i] < CDFmin) { CDFmin = CDF[i]; }
		//std::cout << CDF[i] << std::endl;
	}

	// cdf-norm
	for (int i = 0; i < 256; i++) {
		histE[i] = min((max(round(((CDF[i] - CDFmin) * 255) / ((imgHeight *imgWidth) - CDFmin)), 0)), 255);
	}

	/* Second kernel --------------------------------------------------------------------------*/
	if (m_Context.BuildKernels(kernelProgram2, kernels)) {
		cl_int result = CL_SUCCESS;

		cl::NDRange rangeOffset(0, 0);
		cl::NDRange rangeGlobal(imgWidth, imgHeight);
		cl::NDRange rangeLocal(16, 8);

		cl::Buffer bufferIn((cl::Context)m_Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, cl::size_type(pixelsInBytes), pixelDataSource);
		cl::Buffer bufferOut((cl::Context)m_Context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, cl::size_type(pixelsInBytes), pixelDataDestination);
		cl::Buffer bufCumu((cl::Context)m_Context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, cl::size_type(Size * sizeof(cl_int)), &histE);

		char *mappedPointerIn = (char*)clQueue.enqueueMapBuffer(bufferIn, CL_TRUE, CL_MAP_WRITE, 0, pixelsInBytes, 0, 0, &result);	// expicit upload
		clQueue.finish();

		if (result == CL_SUCCESS) {
			memcpy(mappedPointerIn, pixelDataSource, pixelsInBytes);
			cl::Event endEvent;
			result = clQueue.enqueueUnmapMemObject(bufferIn, mappedPointerIn, 0, &endEvent);
			if (result == CL_SUCCESS) { endEvent.wait(); }
		} else { std::cout << "Map input buffer:" << ClErrorToString(result) << std::endl; }

		// define functor
		auto equalization = cl::KernelFunctor< cl::Buffer&, cl::Buffer&, cl::Buffer&>(kernels, "equalization");
		cl::Event event = equalization(cl::EnqueueArgs(rangeGlobal, rangeLocal), bufferIn, bufferOut, bufCumu, result);

		// if success, download results to host
		if (result == CL_SUCCESS) {
			// copy back the result buffer from the device to the host image buffer			
			char *mappedPointer = (char*)clQueue.enqueueMapBuffer(bufferOut, CL_TRUE, CL_MAP_READ, 0, pixelsInBytes, 0, 0, &result);
			if (result == CL_SUCCESS) {
				memcpy(pixelDataDestination, mappedPointer, pixelsInBytes);
				cl::Event endEvent;
				result = clQueue.enqueueUnmapMemObject(bufferOut, mappedPointer, 0, &endEvent);
				if (result == CL_SUCCESS) { endEvent.wait(); }
			}
		} else {
			std::cout << "Problem invoking kernel equalization;" << std::endl;
			std::cout << " error " << ClErrorToString(result) << std::endl;
			std::cout << " error code: " << (int)result << std::endl;
		}
	} else {
		std::cout << "Problem building kernel equalization;" << std::endl;
		std::cout << " build info:" << m_Context.GetBuildOutput() << std::endl;
	}

	return destinationImage;
}