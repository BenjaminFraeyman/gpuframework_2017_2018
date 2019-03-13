#include "ED_GPU.h"

Image* Filters::ED_GPU::Filter(Image* sourceImageOne) {
	Image* destinationImage = new Image(sourceImageOne->GetWidth(), sourceImageOne->GetHeight());

	unsigned int imgWidth = sourceImageOne->GetWidth();
	unsigned int imgHeight = sourceImageOne->GetHeight();
	unsigned char* pixelDataSource = sourceImageOne->GetPixelData();
	unsigned char* pixelDataDestination = destinationImage->GetPixelData();
	const size_t pixelsInBytes = imgWidth * imgHeight * 4 * sizeof(char);

	const cl_int Size = 9;
	signed int CF[Size] = { -1,0,1,-2,0,2,-1,0,1 };
	signed int CF2[Size] = { 1,2,1,0,0,0,-1,-2,-1 };
	std::vector<int> A(Size, 0);
	std::vector<int> B(Size, 0);
	for (int index = 0; index < Size; ++index) { A[index] = CF[index]; B[index] = CF2[index]; }

	std::string kernelProgram(
		"       __kernel void ED(__global const int* inputImage, __global int* outputImage, __global const int* filter, __global const int* filter2){"
		""
		"		const size_t rowcol = (get_global_id(1) * get_global_size(0) + get_global_id(0));"		// linear address of the pixel
		""
		"		 __local int localBuffer[3][3];"
		""
		"		localBuffer[get_local_id(0)][get_local_id(1)] = inputImage[get_global_id(1) * get_global_size(0) + get_global_id(0)]; "
		""
		"		int colorleftup = inputImage[(get_global_id(1)-1) * get_global_size(0) + get_global_id(0)-1]; " 
		"		int colorleft =  inputImage[(get_global_id(1)) * get_global_size(0) + get_global_id(0)-1];"
		"		int colorleftdown =   inputImage[(get_global_id(1)+1) * get_global_size(0) + get_global_id(0)-1];"

		"		int colorup = inputImage[(get_global_id(1)-1) * get_global_size(0) + get_global_id(0)]; "
		"		int color = localBuffer[get_local_id(0)][get_local_id(1)] ;" //deze is wel correct
		"		int colordown =  inputImage[(get_global_id(1)+1) * get_global_size(0) + get_global_id(0)];"

		"		int colorrightup = inputImage[(get_global_id(1)-1) * get_global_size(0) + get_global_id(0)+1]; "
		"		int colorright = inputImage[(get_global_id(1)) * get_global_size(0) + get_global_id(0)+1]; "
		"		int colorrightdown =   inputImage[(get_global_id(1)+1) * get_global_size(0) + get_global_id(0)+1];"
		""
		"		int r_h = (int)(filter[0]*((colorleftup &0xFF0000) >>16) + filter[1] *((colorup &0xFF0000) >>16) + filter[2]* ((colorrightup &0xFF0000) >>16) "
		"							+filter[3] *((colorleft &0xFF0000) >>16) +filter[4]*((color &0xFF0000) >>16) + filter[5] *((colorright &0xFF0000) >>16)"
		"							+filter[6] *((colorleftdown &0xFF0000) >>16) +filter[7]*((colordown &0xFF0000) >>16) + filter[8] *((colorrightdown &0xFF0000) >>16));"
		""
		"		int g_h = (int)(filter[0]*((colorleftup &0x00FF00) >>8) + filter[1] *((colorup &0x00FF00) >>8) + filter[2]* ((colorrightup &0x00FF00) >>8) "
		"							+filter[3] *((colorleft &0x00FF00) >>8) +filter[4]*((color &0x00FF00) >>8) + filter[5] *((colorright &0x00FF00) >>8)"
		"							+filter[6] *((colorleftdown &0x00FF00) >>8) +filter[7]*((colordown &0x00FF00) >>8) + filter[8] *((colorrightdown &0x00FF00) >>8));"
		""
		"		int b_h = (int)(filter[0]*((colorleftup &0x0000FF) >>0) + filter[1] *((colorup &0x0000FF) >>0) + filter[2]* ((colorrightup &0x0000FF) >>0) "
		"							+filter[3] *((colorleft &0x0000FF) >>0) +filter[4]*((color &0x0000FF) >>0) + filter[5] *((colorright &0x0000FF) >>0)"
		"							+filter[6] *((colorleftdown &0x0000FF) >>0) +filter[7]*((colordown &0x0000FF) >>0) + filter[8] *((colorrightdown &0x0000FF) >>0));"
		""
		"		r_h = min((max( r_h , 0)), 255);"
		"		g_h =min((max( g_h , 0)), 255);"
		"		b_h = min((max( b_h , 0)), 255);"
		""
		""
		"		int r_v = (int)(filter2[0]*((colorleftup &0xFF0000) >>16) + filter2[1] *((colorup &0xFF0000) >>16) + filter2[2]* ((colorrightup &0xFF0000) >>16) "
		"							+filter2[3] *((colorleft &0xFF0000) >>16) +filter2[4]*((color &0xFF0000) >>16) + filter2[5] *((colorright &0xFF0000) >>16)"
		"							+filter2[6] *((colorleftdown &0xFF0000) >>16) +filter2[7]*((colordown &0xFF0000) >>16) + filter2[8] *((colorrightdown &0xFF0000) >>16));"
		""
		"		int g_v = (int)(filter2[0]*((colorleftup &0x00FF00) >>8) + filter2[1] *((colorup &0x00FF00) >>8) + filter2[2]* ((colorrightup &0x00FF00) >>8) "
		"							+filter2[3] *((colorleft &0x00FF00) >>8) +filter2[4]*((color &0x00FF00) >>8) + filter2[5] *((colorright &0x00FF00) >>8)"
		"							+filter2[6] *((colorleftdown &0x00FF00) >>8) +filter2[7]*((colordown &0x00FF00) >>8) + filter2[8] *((colorrightdown &0x00FF00) >>8));"
		""
		"		int b_v = (int)(filter2[0]*((colorleftup &0x0000FF) >>0) + filter2[1] *((colorup &0x0000FF) >>0) + filter2[2]* ((colorrightup &0x0000FF) >>0) "
		"							+filter2[3] *((colorleft &0x0000FF) >>0) +filter2[4]*((color &0x0000FF) >>0) + filter2[5] *((colorright &0x0000FF) >>0)"
		"							+filter2[6] *((colorleftdown &0x0000FF) >>0) +filter2[7]*((colordown &0x0000FF) >>0) + filter2[8] *((colorrightdown &0x0000FF) >>0));"
		""
		"		r_v = min((max( r_v , 0)), 255);"
		"		g_v =min((max( g_v , 0)), 255);"
		"		b_v = min((max( b_v , 0)), 255);"
		""
		"		int red   = ((int)  sqrt(pow(r_h,2)+pow(r_v,2)) << 16 ) & 0xFF0000;"
		"		int green = ((int)  sqrt(pow(g_h,2)+pow(g_v,2)) << 8 )  & 0x00FF00;"
		"		int blue  = ((int)  sqrt(pow(b_h,2)+pow(b_v,2)) << 0 )  & 0x0000FF;"
		""
		"		int alpha =  0xFF000000;"
		"		int newColor = alpha | red | green | blue;"
		"		outputImage[rowcol] = newColor;"
		"	}"
	);

	cl::Program kernels;
	cl::CommandQueue clQueue = (cl::CommandQueue)m_Context;

	if (m_Context.BuildKernels(kernelProgram, kernels)) {
		cl_int result = CL_SUCCESS;

		// define work-items and workgroup 
		cl::NDRange computeDomain(16, 12);
		cl::NDRange workgroupSize(16, 8);
		cl::NDRange rangeOffset(0, 0);
		cl::NDRange rangeGlobal(imgWidth, imgHeight);
		cl::NDRange rangeLocal(16, 2, 0);

		// create buffers and upload all buffers
		cl::Buffer bufA(begin(A), end(A), true);
		cl::Buffer bufB(begin(B), end(B), true);
		cl::Buffer bufferIn((cl::Context)m_Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, cl::size_type(pixelsInBytes), pixelDataSource);			// no upload!
		cl::Buffer bufferOut((cl::Context)m_Context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, cl::size_type(pixelsInBytes), pixelDataDestination);		// no upload required (will store the result later)

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
		auto ED = cl::KernelFunctor< cl::Buffer&, cl::Buffer&, cl::Buffer&, cl::Buffer&>(kernels, "ED");

		// execute functor ( == kernel ED) with input arguments and output argument 
		cl::Event event = ED(cl::EnqueueArgs(rangeGlobal, workgroupSize), bufferIn, bufferOut, bufA, bufB, result);

		// if success, download results to host
		if (result == CL_SUCCESS) {
			// copy back the result buffer form the device to the host image buffer			
			char *mappedPointer = (char*)clQueue.enqueueMapBuffer(bufferOut, CL_TRUE, CL_MAP_READ, 0, pixelsInBytes, 0, 0, &result);
			if (result == CL_SUCCESS) {
				memcpy(pixelDataDestination, mappedPointer, pixelsInBytes);
				cl::Event endEvent;
				result = clQueue.enqueueUnmapMemObject(bufferOut, mappedPointer, 0, &endEvent);
				if (result == CL_SUCCESS)
				{
					endEvent.wait();
				}
			}

		}
		else {
			std::cout << "Problem invoking kernel ED;" << std::endl;
			std::cout << " error " << ClErrorToString(result) << std::endl;
			std::cout << " error code: " << (int)result << std::endl;
		}
	} else {
		std::cout << "Problem building kernel ED;" << std::endl;
		std::cout << " build info:" << m_Context.GetBuildOutput() << std::endl;
	}
	return destinationImage;
}