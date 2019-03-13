#include "ExamplesCL_C_API.h"

#define MEM_SIZE (128)
#define MAX_SOURCE_SIZE (0x100000)

char* kernelString[] = {
	"	__kernel void helloWorld(__global char* outputString){ \n						"		\
	"		outputString[0] = 'H';														"		\
	"		outputString[1] = 'e';														"		\
	"		outputString[2] = 'l';														"		\
	"		outputString[3] = 'l';														"		\
	"		outputString[4] = 'o';														"		\
	"		outputString[5] = ',';														"		\
	"		outputString[6] = ' ';														"		\
	"		outputString[7] = 'W';														"		\
	"		outputString[8] = 'o';														"		\
	"		outputString[9] = 'r';														"		\
	"		outputString[10] = 'l';														"		\
	"		outputString[11] = 'd';														"		\
	"		outputString[12] = '!';														"		\
	"		outputString[13] = '\\0';													"		\
	"	}																				"		
};

void Examples::RunBasicCLExample_C_API()
{

	cl_device_id device_id = NULL;
	cl_context context = NULL;
	cl_command_queue command_queue = NULL;
	cl_mem memobj = NULL;
	cl_program program = NULL;
	cl_kernel kernel = NULL;
	cl_platform_id platform_id = NULL;
	cl_uint ret_num_devices;
	cl_uint ret_num_platforms;
	cl_int ret;

	char string[MEM_SIZE];

	/* Get Platform and Device Info */
	ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
	ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);

	std::cout << "Platform:" << ClErrorToString(ret) << std::endl;

	/* Create OpenCL context */
	//context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
	// Next, create an OpenCL context on the platform.  Attempt to
	// create a GPU-based context, and if that fails, try to create
	// a CPU-based context.
	cl_context_properties contextProperties[] =
	{
		CL_CONTEXT_PLATFORM,
		(cl_context_properties)platform_id,
		0
	};
	context = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_GPU, NULL, NULL, &ret);
	if (ret != CL_SUCCESS)
	{
		std::cout << "GPU Context:" << ClErrorToString(ret) << std::endl;		
		context = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_CPU, NULL, NULL, &ret);
		if (ret != CL_SUCCESS)
		{
			std::cout << "CPU Context:" << ClErrorToString(ret) << std::endl;
			return;
		}
	}

	size_t bufferSize = 0;
	char bufferN[256];
	char bufferV[256];
	char bufferW[256];
	char bufferP[256];
	char bufferE[256];
	clGetPlatformInfo(platform_id, CL_PLATFORM_NAME, 256,		bufferN, &bufferSize);	bufferN[bufferSize] = 0;				
	clGetPlatformInfo(platform_id, CL_PLATFORM_VENDOR, 256,		bufferV, &bufferSize);	bufferV[bufferSize] = 0;
	clGetPlatformInfo(platform_id, CL_PLATFORM_VERSION, 256,	bufferW, &bufferSize);	bufferW[bufferSize] = 0;
	clGetPlatformInfo(platform_id, CL_PLATFORM_PROFILE, 256,	bufferP, &bufferSize);	bufferP[bufferSize] = 0;
	clGetPlatformInfo(platform_id, CL_PLATFORM_EXTENSIONS, 256, bufferE, &bufferSize);	bufferE[bufferSize] = 0;	

	std::cout << "Platform name:       " << bufferN << std::endl;
	std::cout << "Platform vendor:     " << bufferV << std::endl;
	std::cout << "Platform version:    " << bufferW << std::endl;
	std::cout << "Platform profile:    " << bufferP << std::endl;
	std::cout << "Platform extensions: " << bufferE << std::endl;

	clGetDeviceInfo(device_id, CL_DEVICE_NAME, 256, bufferN, &bufferSize);			bufferN[bufferSize] = 0;
	clGetDeviceInfo(device_id, CL_DEVICE_VENDOR, 256, bufferV, &bufferSize);		bufferV[bufferSize] = 0;
	clGetDeviceInfo(device_id, CL_DEVICE_VERSION, 256, bufferW, &bufferSize);		bufferW[bufferSize] = 0;
	clGetDeviceInfo(device_id, CL_DEVICE_PROFILE, 256, bufferP, &bufferSize);		bufferP[bufferSize] = 0;
	clGetDeviceInfo(device_id, CL_DEVICE_EXTENSIONS, 256, bufferE, &bufferSize);	bufferE[bufferSize] = 0;

	std::cout << "Device name:       " << bufferN << std::endl;
	std::cout << "Device vendor:     " << bufferV << std::endl;
	std::cout << "Device version:    " << bufferW << std::endl;
	std::cout << "Device profile:    " << bufferP << std::endl;
	std::cout << "Device extensions: " << bufferE << std::endl;

	/* Create Command Queue */
	command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
	std::cout << "Queue:" << ClErrorToString(ret) << std::endl;

	/* Create Memory Buffer */
	memobj = clCreateBuffer(context, CL_MEM_READ_WRITE, MEM_SIZE * sizeof(char), NULL, &ret);
	std::cout << "Buffer:" << ClErrorToString(ret) << std::endl;

	/* Create Kernel Program from the source */
	
	program = clCreateProgramWithSource(context, 1, (const char **)kernelString, NULL, &ret);
	std::cout << "Program:" << ClErrorToString(ret) << std::endl;

	/* Build Kernel Program */
	ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
	std::cout << "ProgramBuild:" << ClErrorToString(ret) << std::endl;

	/* Create OpenCL Kernel */
	kernel = clCreateKernel(program, "helloWorld", &ret);
	std::cout << "Kernel:" << ClErrorToString(ret) << std::endl;

	/* Set OpenCL Kernel Parameters */
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&memobj);
	std::cout << "KernelArgs:" << ClErrorToString(ret) << std::endl;

	/* Execute OpenCL Kernel */
	ret = clEnqueueTask(command_queue, kernel, 0, NULL, NULL);
	std::cout << "Queuekernel:" << ClErrorToString(ret) << std::endl;

	/* Copy results from the memory buffer */
	ret = clEnqueueReadBuffer(command_queue, memobj, CL_TRUE, 0, MEM_SIZE * sizeof(char), string, 0, NULL, NULL);
	std::cout << "ReadBuffer:" << ClErrorToString(ret) << std::endl;

	/* Display Result */
	std::cout << string << std::endl;	

	/* Finalization */
	ret = clFlush(command_queue);
	ret = clFinish(command_queue);
	ret = clReleaseKernel(kernel);
	ret = clReleaseProgram(program);
	ret = clReleaseMemObject(memobj);
	ret = clReleaseCommandQueue(command_queue);
	ret = clReleaseContext(context);
}
