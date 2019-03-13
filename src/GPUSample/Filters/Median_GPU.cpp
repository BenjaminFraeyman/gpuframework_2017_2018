#include "Median_GPU.h"

Image* Filters::Median_GPU::Filter(Image* sourceImageOne) {
	Image* destinationImage = new Image(sourceImageOne->GetWidth(), sourceImageOne->GetHeight());

	unsigned int imgWidth = sourceImageOne->GetWidth();
	unsigned int imgHeight = sourceImageOne->GetHeight();
	unsigned char* pixelDataSource = sourceImageOne->GetPixelData();
	unsigned char* pixelDataDestination = destinationImage->GetPixelData();
	const size_t pixelsInBytes = imgWidth * imgHeight * 4 * sizeof(char);


	std::string kernelProgram(
	"       __kernel void Median(__global const int* inputImage, __global int* outputImage){"
	"		const size_t rowcol = (get_global_id(1) * get_global_size(0) + get_global_id(0));"
	""
	"		 __local int localBuffer[3][3];"
	""
	"		localBuffer[get_local_id(0)][get_local_id(1)] = inputImage[get_global_id(1) * get_global_size(0) + get_global_id(0)]; "
	""
	"		int colorleftup = inputImage[(get_global_id(1)-1) * get_global_size(0) + get_global_id(0)-1]; "
	"		int colorleft =  inputImage[(get_global_id(1)) * get_global_size(0) + get_global_id(0)-1];"
	"		int colorleftdown =   inputImage[(get_global_id(1)+1) * get_global_size(0) + get_global_id(0)-1];"
	""
	"		int colorup = inputImage[(get_global_id(1)-1) * get_global_size(0) + get_global_id(0)]; "
	"		int color = localBuffer[get_local_id(0)][get_local_id(1)] ;"
	"		int colordown =  inputImage[(get_global_id(1)+1) * get_global_size(0) + get_global_id(0)];"
	""
	"		int colorrightup = inputImage[(get_global_id(1)-1) * get_global_size(0) + get_global_id(0)+1]; "
	"		int colorright = inputImage[(get_global_id(1)) * get_global_size(0) + get_global_id(0)+1]; "
	"		int colorrightdown =   inputImage[(get_global_id(1)+1) * get_global_size(0) + get_global_id(0)+1];"
	""
	"		int redresult[9] = {((colorleftup &0xFF0000) >>16),((colorup &0xFF0000) >>16),((colorrightup &0xFF0000) >>16), ((colorleft &0xFF0000) >>16),((color &0xFF0000) >>16),((colorright &0xFF0000) >>16), ((colorleftdown &0xFF0000) >>16) ,((colordown &0xFF0000) >>16),((colorrightdown &0xFF0000) >>16) };"
	""
	"		for (int i = 0; i < 5; i++) {"
	"			for (int j = i + 1; j < 9; j++) {"
	"				if (redresult[i] > redresult[j]) {"
	"					int tmp = redresult[i];"
	"					redresult[i] = redresult[j];"
	"					redresult[j] = tmp;"
	"				}"
	"			}"
	"		}"
	""
	"		int greenresult[9] ={((colorleftup &0x00FF00) >>8),((colorup &0x00FF00) >>8),((colorrightup &0x00FF00) >>8), ((colorleft &0x00FF00) >>8),((color &0x00FF00) >>8),((colorright &0x00FF00) >>8), ((colorleftdown &0x00FF00) >>8),((colordown &0x00FF00) >>8),((colorrightdown &0x00FF00) >>8) };"
	""
	"		for (int i = 0; i < 5; i++) {"
	"			for (int j = i + 1; j < 9; j++) {"
	"				if (greenresult[i] > greenresult[j]) {"
	"					int tmp = greenresult[i];"
	"					greenresult[i] = greenresult[j];"
	"					greenresult[j] = tmp;"
	"				}"
	"			}"
	"		}"
	""
	"		int blueresult[9] = {((colorleftup &0x0000FF) >>0),((colorup &0x0000FF) >>0),((colorrightup &0x0000FF) >>0), ((colorleft &0x0000FF) >>0),((color &0x0000FF) >>0),((colorright &0x0000FF) >>0), ((colorleftdown &0x0000FF) >>0),((colordown &0x0000FF) >>0),((colorrightdown &0x0000FF) >>0) };"
	""
	"		for (int i = 0; i < 5; i++) {"
	"			for (int j = i + 1; j < 9; j++) {"
	"				if (blueresult[i] > blueresult[j]) {"
	"					int tmp = blueresult[i];"
	"					blueresult[i] = blueresult[j];"
	"					blueresult[j] = tmp;"
	"				}"
	"			}"
	"		}"
	""
	"		int red   = ((int)  redresult[4] << 16 ) & 0xFF0000;"
	"		int green = ((int)   greenresult[4] << 8 )  & 0x00FF00;"
	"		int blue  = ((int)  blueresult[4] << 0 )  & 0x0000FF;"
	""
	"		int alpha =  0xFF000000;"
	"		int newColor = alpha | red | green | blue;"
	"		outputImage[rowcol] = newColor;"
	"	}"
	);

	cl::Program kernels;
	cl::CommandQueue clQueue = (cl::CommandQueue)m_Context;

	if (m_Context.BuildKernels(kernelProgram, kernels))
	{
		cl_int result = CL_SUCCESS;

		// define work-items and workgroup 
		cl::NDRange computeDomain(imgWidth, imgWidth);
		cl::NDRange workgroupSize(4, 4);
		cl::NDRange rangeOffset(0, 0);
		cl::NDRange rangeGlobal(imgWidth, imgHeight);
		cl::NDRange rangeLocal(16, 2, 0);

		// create buffers and upload all buffers
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
		auto Median = cl::KernelFunctor< cl::Buffer&, cl::Buffer&>(kernels, "Median");

		// execute functor ( == kernel Median) with input arguments and output argument 
		cl::Event event = Median(cl::EnqueueArgs(rangeGlobal, workgroupSize), bufferIn, bufferOut, result);

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
			std::cout << "Problem invoking kernel Median;" << std::endl;
			std::cout << " error " << ClErrorToString(result) << std::endl;
			std::cout << " error code: " << (int)result << std::endl;
		}
	}
	else
	{
		std::cout << "Problem building kernel Median;" << std::endl;
		std::cout << " build info:" << m_Context.GetBuildOutput() << std::endl;
	}

	return destinationImage;
}