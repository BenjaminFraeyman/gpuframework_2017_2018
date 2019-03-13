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

	std::vector<float> matrix1(9, 0);
	matrix1[0] = -1.0f;
	matrix1[1] = 0.0f;
	matrix1[2] = 1.0f;
	matrix1[3] = -2.0f;
	matrix1[4] = 0.0f;
	matrix1[5] = 2.0f;
	matrix1[6] = -1.0f;
	matrix1[7] = 0.0f;
	matrix1[8] = 1.0f;

	std::vector<float> matrix2(9, 0);
	matrix1[0] = 1.0f;
	matrix1[1] = 2.0f;
	matrix1[2] = 1.0f;
	matrix1[3] = 0.0f;
	matrix1[4] = 0.0f;
	matrix1[5] = 0.0f;
	matrix1[6] = -1.0f;
	matrix1[7] = -2.0f;
	matrix1[8] = -1.0f;

	std::string kernelProgram(
		"       int getColor(__global int *arr, int x, int y, int w, int h) {"
		"           x = (x < 0 ? 0 : (x > w - 1 ? w - 1 : x));"
		"           y = (y < 0 ? 0 : (y > h - 1 ? h - 1 : y));"
		"           return arr[(y * w + x)];"
		"       }"
		"       __kernel void ED(__global const int* inputImage, __global int* outputImage, global const float* matrix1, global const float* matrix2){"
		""
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
		"           float yRedDestination = 0;"
		"           float yGreenDestination = 0;"
		"           float yBlueDestination = 0;"
		"           x = x + 1;"
		"           y = y + 1;"
		"           for(int i = 0; i < 3; ++i) {"
		"               for(int j = 0; j < 3; ++j) {"
		"                   xRedDestination += matrix1[j * 3 + i] * ((pixelBuffer[x + (i - 1)][y + (j - 1)] & 0x00FF0000) >> 16);"
		"                   xGreenDestination += matrix1[j * 3 + i] * ((pixelBuffer[x + (i - 1)][y + (j - 1)] & 0x0000FF00) >> 8);"
		"                   xBlueDestination += (matrix1[j * 3 + i] * ((pixelBuffer[x + (i - 1)][y + (j - 1)] & 0x000000FF) >> 0));"
		"                   yRedDestination += matrix2[j * 3 + i] * ((pixelBuffer[x + (i - 1)][y + (j - 1)] & 0x00FF0000) >> 16);"
		"                   yGreenDestination += matrix2[j * 3 + i] * ((pixelBuffer[x + (i - 1)][y + (j - 1)] & 0x0000FF00) >> 8);"
		"                   yBlueDestination += (matrix2[j * 3 + i] * ((pixelBuffer[x + (i - 1)][y + (j - 1)] & 0x000000FF) >> 0));"
		"               }"
		"           }"
		""
		"           float redRoot = sqrt(pown(xRedDestination, 2) + pown(yRedDestination,2));"
		"           float greenRoot = sqrt(pown(xGreenDestination, 2) + pown(yGreenDestination,2));"
		"           float blueRoot = sqrt(pown(xBlueDestination, 2) + pown(yBlueDestination,2));"
		""
		"           redRoot = fmin(fmax(0.0,redRoot), 255.0);"
		"           greenRoot = fmin(fmax(0.0, greenRoot), 255.0);"
		"           blueRoot = fmin(fmax(0.0, blueRoot), 255.0);"
		""
		"           int alpha = 0xFF << 24;"
		"           int red = (int)(redRoot) << 16;"
		"           int green = (int)(greenRoot) << 8;"
		"           int blue = (int)(blueRoot) << 0;"
		"           int color = alpha | red | green | blue;"
		"           outputImage[rowCol] = color;"
		"	    }"
		//"		int w = 3;"
		//"		int wBy2 = w>>1;"//w divided by 2
		//""
		//"		int x = get_global_id(0);"
		//"		int y = get_global_id(1);"
		//"		const int Block_dim = 16;"// tilesize
		//"		int i = get_group_id(0);int j = get_group_id(1);" // id of work item
		//"		int idX = get_local_id(0); int idY = get_local_id(1);"
		//"		int ii = i*get_local_size(0) + idX;int jj = j*get_local_size(1) + idY;"// get_global_id
		//""
		//"		__local int localBuffertest[18][4];" // id of work group
		//"		localBuffertest[idX][idY] = inputImage[jj * get_global_size(0) + ii];" // read pixels
		//"		int2 coords = (int2)(ii,jj);"
		//"		if(idX < w){"
		//"			coords.x = ii + get_local_size(0); coords.y = jj;"
		//"			localBuffertest[idX + get_local_size(0)][idY] = inputImage[coords.y * get_global_size(0) + coords.x];"
		//"		}"
		//"		if(idY < w){"
		//"			coords.x = ii; coords.y = jj + get_local_size(1);"
		//"			localBuffertest[idX][idY + get_local_size(1)] = inputImage[coords.y * get_global_size(0) + coords.x];"
		//"		}"
		//"		if(idY < w && idX < w){"
		//"			coords.x = ii + get_local_size(0); coords.y = jj + get_local_size(1);"
		//"			localBuffertest[idX + get_local_size(0)][idY + get_local_size(1)] = inputImage[coords.y * get_global_size(0) + coords.x];"
		//"		}"
		//"		barrier(CLK_LOCAL_MEM_FENCE);"
		//"		int tempredx=0;"
		//"		int tempgreenx=0;"
		//"		int tempbluex=0;"
		//"		int tempredy=0;"
		//"		int tempgreeny=0;"
		//"		int tempbluey=0;"
		////"		if(x >= 0 && y >= 0 && x < 2560 && y < 1600){" // if in pic bounds
		//"			for(int ix = 0; ix < w; ix++){"
		//"				for(int jy = 0; jy < w; jy++){"
		//"					tempredx += ((int)(localBuffertest[ix+idX][jy+idY] & 0xFF0000) >> 16)* filter[ix + w * jy]; "
		//"					tempgreenx += ((int)(localBuffertest[ix+idX][jy+idY] & 0x00FF00) >>8)* filter[ix + w * jy]; "
		//"					tempbluex += ((int)(localBuffertest[ix+idX][jy+idY] &0x0000FF) >>0)* filter[ix + w * jy]; "
		//"					tempredy += ((int)(localBuffertest[ix + idX][jy + idY] & 0xFF0000) >> 16)* filter2[ix + w * jy]; "
		//"					tempgreeny += ((int)(localBuffertest[ix+idX][jy+idY] & 0x00FF00) >>8)* filter2[ix + w * jy]; "
		//"					tempbluey += ((int)(localBuffertest[ix+idX][jy+idY] &0x0000FF) >>0)* filter2[ix + w * jy]; "
		//"				}"
		//"			}"
		////"			barrier(CLK_LOCAL_MEM_FENCE);"
		//""
		//"			int redx = min((max(tempredx, 0)), 255);"
		//"			int greenx = min((max(tempgreenx, 0)), 255);"
		//"			int bluex = min((max(tempbluex, 0)), 255);"
		//"			int redy = min((max(tempredy, 0)), 255); "
		//"			int greeny = min((max(tempgreeny, 0)), 255);"
		//"			int bluey = min((max(tempbluey, 0)), 255);"
		//""
		//"			int red   = ((int)  sqrt(pow(redx,2)+pow(redy,2)) << 16 ) & 0xFF0000;"
		//"			int green = ((int)  sqrt(pow(greenx,2)+pow(greeny,2)) << 8 )  & 0x00FF00;"
		//"			int blue  = ((int)  sqrt(pow(bluex,2)+pow(bluey,2)) << 0 )  & 0x0000FF;"
		//""
		//"			int newColor = 0xFF000000 | red | green | blue;"
		////"			barrier(CLK_LOCAL_MEM_FENCE);"
		////"			coords = (int2)(ii + wBy2, jj + wBy2);"
		//"			outputImage[get_global_id(1) * get_global_size(0) + get_global_id(0)] = newColor;"
		////"		}"
		//"	}"
	);

	cl::Program kernels;
	cl::CommandQueue clQueue = (cl::CommandQueue)m_Context;

	if (m_Context.BuildKernels(kernelProgram, kernels)) {
		cl_int result = CL_SUCCESS;

		// define work-items and workgroup 
		//cl::NDRange computeDomain(imgWidth, imgHeight);
		//cl::NDRange workgroupSize(16, 16);
		cl::NDRange rangeOffset(0, 0);
		cl::NDRange rangeGlobal(imgWidth, imgHeight);
		cl::NDRange rangeLocal(16, 2, 0);

		// create buffers and upload all buffers
		cl::Buffer bufA(begin(matrix1), end(matrix1), true);
		cl::Buffer bufB(begin(matrix2), end(matrix2), true);
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
		cl::Event event = ED(cl::EnqueueArgs(rangeGlobal, rangeLocal), bufferIn, bufferOut, bufA, bufB, result);

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