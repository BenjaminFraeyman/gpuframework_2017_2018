#include "EdgeFilterGPU.h"

Image* Filters::EdgeFilterGPU::Filter(Image* sourceImageOne)
{

    Image* destinationImage = new Image(sourceImageOne->GetWidth(), sourceImageOne->GetHeight());

    unsigned int imgWidth = sourceImageOne->GetWidth();
    unsigned int imgHeight = sourceImageOne->GetHeight();
    unsigned char* pixelDataSource = sourceImageOne->GetPixelData();
    unsigned char* pixelDataDestination = destinationImage->GetPixelData();

    //cl_int randomTableSize = 71;
    const int pixels = imgWidth * imgHeight;
    const size_t pixelsInBytes = pixels * 4 * sizeof(char);
    //std::vector<float> randomTable(randomTableSize, 0);
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

    // just build a matrix with random numbers
    //for (int index = 0; index < randomTableSize; ++index) { randomTable[index] = 1.0f *  ((rand() % 1000) / 1000.0f); }

    std::string kernelProgram(
            "       __kernel void gpuFilter(__global const int* inputImage, __global int* outputImage, global const float* matrix1, global const float* matrix2){		"
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
                    "       int getColor(int *arr, int x, int y, int w, int h) {"
                    "           x = (x < 0 ? 0 : (x > w - 1 ? w - 1 : x));"
                    "           y = (y < 0 ? 0 : (y > h - 1 ? h - 1 : y));"
                    "           return arr[(y * w + x)];"
                    "       }"
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
        cl::Buffer bufferMatrix1(begin(matrix1), end(matrix1), true);
        cl::Buffer bufferMatrix2(begin(matrix2), end(matrix2), true);// implicit upload
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
        auto			  gpuFilter = cl::KernelFunctor< cl::Buffer&, cl::Buffer&, cl::Buffer&, cl::Buffer&>(kernels, "gpuFilter");

        // execute functor ( == kernel noise) with input arguments and output argument
        cl::Event event = gpuFilter(cl::EnqueueArgs(rangeGlobal, rangeLocal), bufferIn, bufferOut, bufferMatrix1, bufferMatrix2, result);

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
            std::cout << "Problem invoking kernel gpuFilter;" << std::endl;
            std::cout << " error " << ClErrorToString(result) << std::endl;
            std::cout << " error code: " << (int)result << std::endl;
        }
    }
    else
    {
        std::cout << "Problem building kernel gpuFilter;" << std::endl;
        std::cout << " build info:" << m_Context.GetBuildOutput() << std::endl;
    }

    return destinationImage;
}
