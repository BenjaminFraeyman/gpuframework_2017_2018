#include "ExamplesCL_CPP.h"
#include <math.h>

void Examples::RunBasicCLExample_CPP1(Examples::CLContext& context)
{

	std::string kernelProgram(
		"	__kernel void helloWorld(__global char* outputString){ \n						"
		"		outputString[0] = 'H';														"
		"		outputString[1] = 'e';														"
		"		outputString[2] = 'l';														"
		"		outputString[3] = 'l';														"
		"		outputString[4] = 'o';														"
		"		outputString[5] = ',';														"
		"		outputString[6] = ' ';														"
		"		outputString[7] = 'W';														"
		"		outputString[8] = 'o';														"
		"		outputString[9] = 'r';														"
		"		outputString[10] = 'l';														"
		"		outputString[11] = 'd';														"
		"		outputString[12] = '!';														"
		"		outputString[13] = '\\0';													"
		"	}																				"
	);

	// Receive Hello World! string from the GPU

	cl::Program kernels;
	cl::CommandQueue clQueue = (cl::CommandQueue)context;

	if (context.BuildKernels(kernelProgram, kernels))
	{
		const int numElements = 32;

		// output buffer is filled with 0 values, will be filled by GPU.
		std::vector<char> output(numElements, 0);

		// prepare GPU buffer that maps on output buffer. Indicate if it is read-only or read-write.		
		cl::Buffer outputBuffer(begin(output), end(output), false);
		cl_int result = CL_SUCCESS;

		// build functor type with argument types + kernel
		auto helloWorldKernel =
			cl::KernelFunctor<
			cl::Buffer&
			>(kernels, "helloWorld");

		// execute functor with argument instances
		cl::Event event = helloWorldKernel(
			cl::EnqueueArgs(
				cl::NDRange(numElements)),
			outputBuffer,
			result);

		event.wait();

		if (result == CL_SUCCESS)
		{
			// copy back the result buffer form the device
			cl::copy(outputBuffer, begin(output), end(output));

			// print the output
			std::cout << "Output:\t";
			for (int i = 0; (i < numElements) && (output[i]!='\0'); ++i) 
			{
				std::cout << output[i];
			}
			std::cout << std::endl;
		}
		else
		{
			std::cout << "Problem invoking kernel helloWorld;" << std::endl;
			std::cout << " error " << ClErrorToString(result) << std::endl;
			std::cout << " error code: " << (int)result << std::endl;
		}
	}
}

void Examples::RunBasicCLExample_CPP2(Examples::CLContext& context)
{
	// build kernel
	std::string kernelProgram(
			"	__kernel void matrixTranspose(__global const float* A, __global float* R){ \n				  	"
			"		int rowcol1 =  get_global_id(1)*get_global_size(0) + get_global_id(0);						"
			"		int rowcol2 =  get_global_id(0)*get_global_size(1) + get_global_id(1);						"
			"		R[rowcol1] = A[rowcol2];																	"
			"	}																								"
		);

	cl::Program kernels;
	cl::CommandQueue clQueue = (cl::CommandQueue)context;

	if (context.BuildKernels(kernelProgram, kernels))
	{
		const int matrixW = 8;
		const int matrixH = 8;
		const int numElements = matrixW * matrixH;
		std::vector<float> A(numElements, 0);
		std::vector<float> R(numElements, 0);

		// just build a matrix with random numbers
		for (int index = 0; index < numElements; ++index) { A[index] = (255.0f * ((rand() % 1000) / 1000.0f)); }

		cl::Buffer bufferA(begin(A), end(A), true);
		cl::Buffer bufferR(begin(R), end(R), false);

		cl_int result = CL_SUCCESS;

		cl::NDRange rangeOffset(0, 0);
		cl::NDRange rangeGlobal(matrixW, matrixH);
		cl::NDRange rangeLocal(matrixW, matrixH);

		auto transposeKernel = cl::KernelFunctor< cl::Buffer&, cl::Buffer&	>(kernels, "matrixTranspose");
		cl::Event event = transposeKernel(cl::EnqueueArgs(rangeOffset, rangeGlobal, rangeLocal), bufferA, bufferR, result);

		if (result == CL_SUCCESS)
		{
			// copy back the result buffer form the device
			cl::copy(bufferR, begin(R), end(R));

			// print the output
			std::cout << "Input:\t" << std::endl;
			for (int w = 0; w < matrixW; ++w) {
				for (int h = 0; h < matrixH; ++h) {
					float f = ((float*)&A[0])[h * matrixW + w];
					std::cout << "\t" << f;
				}
				std::cout << std::endl;
			}
			std::cout << std::endl;

			std::cout << "Output:\t" << std::endl;
			for (int w = 0; w < matrixW; ++w) {
				for (int h = 0; h < matrixH; ++h) {
					float fA = ((float*)&A[0])[w * matrixW + h];
					float fR = ((float*)&R[0])[h * matrixW + w];
					if (fA == fR)
						std::cout << "\t" << fR;
					else
						std::cout << "\t" << "failed";
				}
				std::cout << std::endl;
			}
			std::cout << std::endl;
		}
		else
		{
			std::cout << "Problem invoking kernel matrixTranspose" << std::endl;
			std::cout << " error " << ClErrorToString(result) << std::endl;
			std::cout << " error code: " << (int)result << std::endl;
		}
	}

}
