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
		"       __kernel void GB(__global const int* inputImage, __global int* outputImage, __global const float* filter){ "
		"		const size_t rowcol = (get_global_id(1) * get_global_size(0) + get_global_id(0));"
		"		 __local int localBuffer[3][3];"
		"		localBuffer[get_local_id(0)][get_local_id(1)] = inputImage[get_global_id(1) * get_global_size(0) + get_global_id(0)]; "
		""
		"		int colorleftup = inputImage[(get_global_id(1)-1) * get_global_size(0) + get_global_id(0)-1]; " //dit klopt nog niet moet met local
		"		int colorleft =  inputImage[(get_global_id(1)) * get_global_size(0) + get_global_id(0)-1];"
		"		int colorleftdown =   inputImage[(get_global_id(1)+1) * get_global_size(0) + get_global_id(0)-1];"

		"		int colorup = inputImage[(get_global_id(1)-1) * get_global_size(0) + get_global_id(0)]; "
		"		int color = localBuffer[get_local_id(0)][get_local_id(1)] ;" //deze is wel correct
		"		int colordown =  inputImage[(get_global_id(1)+1) * get_global_size(0) + get_global_id(0)];"

		"		int colorrightup = inputImage[(get_global_id(1)-1) * get_global_size(0) + get_global_id(0)+1]; "
		"		int colorright = inputImage[(get_global_id(1)) * get_global_size(0) + get_global_id(0)+1]; "
		"		int colorrightdown =   inputImage[(get_global_id(1)+1) * get_global_size(0) + get_global_id(0)+1];"
		""
		""
		"		int redresult = (int)(filter[0]*((colorleftup &0xFF0000) >>16) + filter[1] *((colorup &0xFF0000) >>16) + filter[2]* ((colorrightup &0xFF0000) >>16) "
		"							+filter[3] *((colorleft &0xFF0000) >>16) +filter[4]*((color &0xFF0000) >>16) + filter[5] *((colorright &0xFF0000) >>16)"
		"							+filter[6] *((colorleftdown &0xFF0000) >>16) +filter[7]*((colordown &0xFF0000) >>16) + filter[8] *((colorrightdown &0xFF0000) >>16) );"
		""
		"		int greenresult = (int)(filter[0]*((colorleftup &0x00FF00) >>8) + filter[1] *((colorup &0x00FF00) >>8) + filter[2]* ((colorrightup &0x00FF00) >>8) "
		"							+filter[3] *((colorleft &0x00FF00) >>8) +filter[4]*((color &0x00FF00) >>8) + filter[5] *((colorright &0x00FF00) >>8)"
		"							+filter[6] *((colorleftdown &0x00FF00) >>8) +filter[7]*((colordown &0x00FF00) >>8) + filter[8] *((colorrightdown &0x00FF00) >>8) );"
		""
		"		int blueresult = (int)(filter[0]*((colorleftup &0x0000FF) >>0) + filter[1] *((colorup &0x0000FF) >>0) + filter[2]* ((colorrightup &0x0000FF) >>0) "
		"							+filter[3] *((colorleft &0x0000FF) >>0) +filter[4]*((color &0x0000FF) >>0) + filter[5] *((colorright &0x0000FF) >>0)"
		"							+filter[6] *((colorleftdown &0x0000FF) >>0) +filter[7]*((colordown &0x0000FF) >>0) + filter[8] *((colorrightdown &0x0000FF) >>0) );"
		""
		"		redresult = min((max( redresult , 0)), 255);"
		"		greenresult =min((max( greenresult , 0)), 255);"
		"		blueresult = min((max( blueresult , 0)), 255);"
		""
		"	int	red   = 		((int)  redresult << 16 ) & 0xFF0000;"
		"	int	  green =  		((int)  greenresult << 8 )  & 0x00FF00;"
		"	int	  blue  = 		((int)  blueresult << 0 )  & 0x0000FF;"
		""
		"		int alpha =  0xFF000000;"
		"		int newColor = alpha | red | green | blue;"
		"		outputImage[rowcol] = newColor;"
		"	}"
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
		cl::NDRange workgroupSize(4, 4);

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
		cl::Event event = GB(cl::EnqueueArgs(rangeGlobal, workgroupSize), bufferIn, bufferOut, bufferFilter, result);

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