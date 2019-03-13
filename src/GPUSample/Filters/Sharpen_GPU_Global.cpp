#include "Sharpen_GPU_Global.h"

Image* Filters::Sharpen_GPU_Global::Filter(Image* sourceImageOne)
{

	Image* destinationImage = new Image(sourceImageOne->GetWidth(), sourceImageOne->GetHeight());

	unsigned int imgWidth = sourceImageOne->GetWidth();
	unsigned int imgHeight = sourceImageOne->GetHeight();
	unsigned char* pixelDataSource = sourceImageOne->GetPixelData();
	unsigned char* pixelDataDestination = destinationImage->GetPixelData();

	signed int filter[10] = { 1 ,0 ,-1 ,0 ,-1 ,5 ,-1 ,0 ,-1 ,0 };

	const int pixels = imgWidth * imgHeight;
	const size_t pixelsInBytes = pixels * 4 * sizeof(char);



	cl_int Size = 10;
	//const int pixels2 = imgWidth * imgHeight;
	//const size_t pixelsInBytes = pixels * 4 * sizeof(char);
	std::vector<int> A(Size, 0);

	// just build a matrix with random numbers
	for (int index = 0; index < Size; ++index) { A[index] = filter[index]; }

	// used links to find out how local memory works
	//https://software.intel.com/en-us/forums/opencl/topic/558984
	//https://stackoverflow.com/questions/12900835/how-can-i-optimize-this-opencl-sobel-filter-kernel
	//https://www.evl.uic.edu/kreda/gpu/image-convolution/
	//http://www.cmsoft.com.br/opencl-tutorial/case-study-high-performance-convolution-using-opencl-__local-memory/
	//https://github.com/juxiangwu/opencl-image-filter/tree/master/kernels
	//https://www.mql5.com/en/articles/407
	//https://github.com/CRVI/OpenCLIPP
	//http://bandi.me/seam-carving/
	//https://cnugteren.github.io/tutorial/pages/page4.html

	std::string kernelProgram(
		"       __kernel void Sharpen(__global const int* inputImage, __global int* outputImage, __global const int* filter){		"
		""
		"		int w = 3;"
		"		int wBy2 = w>>1;"//w divided by 2
		""
		"		const int TS = 32;"// tilesize
		"		int x = get_global_id(0); int y = get_global_id(1);"
		"		int i = get_group_id(0);int j = get_group_id(1);" // id of work item
		"		int xsize = get_global_size(0); int ysize = get_global_size(1);"
		"		int tile_x = get_local_id(0); int tile_y = get_local_id(1);"
		"		int ii = i*16 + tile_x;int jj = j*16 + tile_y;"// get_global_id
		"		int tile_xsize = get_local_size(0); int tile_ysize = get_local_size(1);"
		"		const size_t rowcol = (y * xsize + x);" // linear address of the pixel
		""
		"		__local int localBuffertest[16+6][16+6];" // id of work group
		"		localBuffertest[tile_x][tile_y] = inputImage[get_global_id(1) * get_global_size(0) + get_global_id(0)];" // read pixels
		"		int2 coords = (int2)(ii,jj);"
		"		if(tile_x < w){"
		"			coords.x = ii + 16; coords.y = jj;"
		"			localBuffertest[tile_x + 16][tile_y] = "
		//"		 __local int localBuffer[TS][TS];"
		//"const int tiledRow = TS * get_group_id(0) + x;"
		//"const int tiledCol = TS * get_group_id(1) + y;"
		//"const int index = tiledCol * xsize + tiledRow;"
		//""
		//"		localBuffer[tile_x][tile_y] = inputImage[index]; "
		//""
		//""
		//"	barrier (CLK_LOCAL_MEM_FENCE); 	"
		//""
		//"		int colorleftup = localBuffer[0][0] ; " //dit probeer ik in local memory te hebben
		//"		int colorleft =  localBuffer[1][0];"
		//"		int colorleftdown =   localBuffer[2][0];"

		//"		int colorup = localBuffer[0][1]; "
		//"		int color = localBuffer[tile_x][tile_y] ;" // hier lukte dit
		//"		int colordown =  localBuffer[2][1];"

		//"		int colorrightup = localBuffer[0][2]; "
		//"		int colorright = localBuffer[1][2]; "
		//"		int colorrightdown =   localBuffer[2][2];"
		""
		""// Huidige werkende implementatie met global memory
		"		__local int localBuffer[3][3];"
		"		localBuffer[get_local_id(0)][get_local_id(1)] = inputImage[get_global_id(1) * get_global_size(0) + get_global_id(0)]; "

		""
		"		 int colorleftup = inputImage[(y-1) * xsize + x-1];"
		"		 int colorleft =  inputImage[(y) * xsize + x-1];"
		"		 int colorleftdown =   inputImage[(y+1) * xsize + x-1];"

		"		 int colorup = inputImage[(y-1) * xsize + x]; "
		"		 int color = localBuffer[tile_x][tile_y] ;"
		"		 int colordown =  inputImage[(y+1) * xsize + x];"

		"		int colorrightup = inputImage[(y-1) * xsize + x+1]; "
		"		 int colorright = inputImage[(y) * xsize + x+1]; "
		"		 int colorrightdown =   inputImage[(y+1) * xsize + x+1];"
		""
		""
		""
		""
		"		int redresult = (int)(filter[1]*((colorleftup &0xFF0000) >>16) + filter[2] *((colorup &0xFF0000) >>16) + filter[3]* ((colorrightup &0xFF0000) >>16) "
		"							+filter[4] *((colorleft &0xFF0000) >>16) +filter[5]*((color &0xFF0000) >>16) + filter[6] *((colorright &0xFF0000) >>16)"
		"							+filter[7] *((colorleftdown &0xFF0000) >>16) +filter[8]*((colordown &0xFF0000) >>16) + filter[9] *((colorrightdown &0xFF0000) >>16) )/filter[0];"
		""
		"		int greenresult = (int)(filter[1]*((colorleftup &0x00FF00) >>8) + filter[2] *((colorup &0x00FF00) >>8) + filter[3]* ((colorrightup &0x00FF00) >>8) "
		"							+filter[4] *((colorleft &0x00FF00) >>8) +filter[5]*((color &0x00FF00) >>8) + filter[6] *((colorright &0x00FF00) >>8)"
		"							+filter[7] *((colorleftdown &0x00FF00) >>8) +filter[8]*((colordown &0x00FF00) >>8) + filter[9] *((colorrightdown &0x00FF00) >>8) )/filter[0];"
		""
		"		int blueresult = (int)(filter[1]*((colorleftup &0x0000FF) >>0) + filter[2] *((colorup &0x0000FF) >>0) + filter[3]* ((colorrightup &0x0000FF) >>0) "
		"							+filter[4] *((colorleft &0x0000FF) >>0) +filter[5]*((color &0x0000FF) >>0) + filter[6] *((colorright &0x0000FF) >>0)"
		"							+filter[7] *((colorleftdown &0x0000FF) >>0) +filter[8]*((colordown &0x0000FF) >>0) + filter[9] *((colorrightdown &0x0000FF) >>0) )/filter[0];"
		""
		""
		"		redresult = min((max( redresult , 0)), 255);"
		"		greenresult =min((max( greenresult , 0)), 255);"
		"		blueresult = min((max( blueresult , 0)), 255);"
		""
		""
		""
		"		int  red   = 		((int)  redresult << 16 ) & 0xFF0000;"
		"		int  green =  		((int)  greenresult << 8 )  & 0x00FF00;"
		"		int  blue  = 		((int)  blueresult << 0 )  & 0x0000FF;"
		""
		"		int alpha =  0xFF000000;"
		"		int newColor = alpha | red | green | blue;"
		"		outputImage[rowcol] = newColor;"
		//"		outputImage[index] = localBuffer[tile_x][tile_y];"
		"	}"
	);

	cl::Program kernels;
	cl::CommandQueue clQueue = (cl::CommandQueue)m_Context;

	if (m_Context.BuildKernels(kernelProgram, kernels))
	{
		cl_int result = CL_SUCCESS;
		// define work-items and workgroup 
		cl::NDRange computeDomain(16, 12);
		//cl::NDRange workgroupSize(16, 8);
		cl::NDRange workgroupSize(16, 16);

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
		cl::Event event = Sharpen(cl::EnqueueArgs(rangeGlobal, workgroupSize), bufferIn, bufferOut,  bufA, result);

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