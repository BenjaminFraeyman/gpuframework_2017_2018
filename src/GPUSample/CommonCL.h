#ifndef CL_CONTEXT
#define CL_CONTEXT

#include <sstream>

namespace Examples
{
	class CLContext
	{
		std::vector<cl::Platform> m_Platforms;
		cl::Platform m_Platform;
		cl::Context m_Context;
		cl::CommandQueue m_Queue;
		cl::Device m_Device;
		std::string m_BuildString;
		std::stringstream m_SetupOutput;
		std::stringstream m_BuildOutput;
		std::stringstream m_Description;
		bool m_PlatformDeviceContextFound;
	public:
		CLContext(bool testGPUDevices = true, bool testCPUDevices = true, bool testAccelerators = false, std::string clVersion = "")
		{
			Setup( testGPUDevices, testCPUDevices, testAccelerators, clVersion);
		}

		explicit operator cl::Context()	{		return m_Context;		}
		explicit operator cl::CommandQueue() {	return m_Queue;			}
		explicit operator cl::Platform() {		return m_Platform;		}
		explicit operator cl::Device() {		return m_Device;		}
		explicit operator std::string() {		return m_BuildString;	}
		explicit operator bool() {				return m_PlatformDeviceContextFound; }
		
		std::string GetSetupOutput() const {	return m_SetupOutput.str();	}
		std::string GetBuildOutput() const {	return m_BuildOutput.str(); }
		std::string GetDescription() const {	return m_Description.str(); }

		bool Setup(bool testGPUDevices = true, bool testCPUDevices = true, bool testAccelerators = false, std::string clVersion = "")
		{
			m_SetupOutput.clear();
			bool result = true;
			bool found = false;

			bool clVersionRequested = clVersion != "";
			std::string versionSelected;
			cl::Platform::get(&m_Platforms);
			// loops over all platforms and devices, selects the first eligiable combination of an opencl 1.x platform and a GPU or CPU device that can execute it
			std::vector<std::string> versions = clVersionRequested ? std::vector<std::string>{clVersion} : std::vector<std::string>{ "OpenCL 2.2", "OpenCL 2.1", "OpenCL 2.0", "OpenCL 1.2", "OpenCL 1.1", "OpenCL 1.0" };
			std::vector<cl_device_type> devices;
			if (testGPUDevices) devices.push_back(CL_DEVICE_TYPE_GPU);
			if (testCPUDevices) devices.push_back(CL_DEVICE_TYPE_CPU);
			if (testAccelerators) devices.push_back(CL_DEVICE_TYPE_ACCELERATOR);
			m_SetupOutput <<"Searching: " << (testGPUDevices ? "GPU" : "NOGPU") << " - " << (testCPUDevices ? "CPU" : "NOCPU") << " - " << (testAccelerators ? "ACC" : "NOACC") << std::endl;
			for (auto &p : m_Platforms)
			{
				auto &p = m_Platforms[1];
				m_SetupOutput <<"	testing platform:       " << p.getInfo<CL_PLATFORM_NAME>() << std::endl;
				for (auto &v : versions)
				{					
					std::string pname = p.getInfo<CL_PLATFORM_NAME>();					
					std::string platver = p.getInfo<CL_PLATFORM_VERSION>();
					if ((platver.find(v) != std::string::npos) && (pname.find("Clover") == std::string::npos))
					{
						versionSelected = v;
						m_Platform = p;

						if (m_Platform() != 0)
						{
							for (auto &devType : devices)
							{
								std::vector<cl::Device> devices;
								m_Platform.getDevices(devType, &devices);
								if (devices.size())
								{
									m_Device = devices[0];
									if (m_Device != devices[0])
									{
										m_Device = cl::Device();
									}
									else
									{
										found = true; break;	// we found a good combination
									}
								}
							}
							if (found) break;
						}
					}
				}
				if (found) break;
			}

			if ((m_Platform() == 0) || (m_Device() == 0))
			{
				m_SetupOutput <<"No OpenCL 1.x platform / device combination found.\n";
				return false;
			}			
			
			m_Description.clear();			
			m_Description <<"	Platform name:       " << m_Platform.getInfo<CL_PLATFORM_NAME>()		<< std::endl;
			m_Description <<"	Platform vendor:     " << m_Platform.getInfo<CL_PLATFORM_VENDOR>()		<< std::endl;
			m_Description <<"	Platform version:    " << m_Platform.getInfo<CL_PLATFORM_VERSION>()		<< std::endl;
			m_Description <<"	Platform profile:    " << m_Platform.getInfo<CL_PLATFORM_PROFILE>()		<< std::endl;
			m_Description <<"	Platform extensions: " << m_Platform.getInfo<CL_PLATFORM_EXTENSIONS>()	<< std::endl;

			m_Description <<"	Device name:         " << m_Device.getInfo< CL_DEVICE_NAME             >() << std::endl;
			m_Description <<"	Device vendor:       " << m_Device.getInfo< CL_DEVICE_VENDOR           >() << std::endl;
			m_Description <<"	Device version:      " << m_Device.getInfo< CL_DRIVER_VERSION          >() << std::endl;
			m_Description <<"	Device profile:      " << m_Device.getInfo< CL_DEVICE_PROFILE          >() << std::endl;
			m_Description <<"	Device extensions:   " << m_Device.getInfo< CL_DEVICE_VERSION          >() << std::endl;
			m_Description <<"	Device extensions:   " << m_Device.getInfo< CL_DEVICE_EXTENSIONS       >() << std::endl;
			m_Description <<"	Device extensions:   " << m_Device.getInfo< CL_DEVICE_PLATFORM         >() << std::endl;
			m_Description <<"	Device extensions:   " << m_Device.getInfo< CL_DEVICE_DOUBLE_FP_CONFIG >() << std::endl;

			m_SetupOutput << "Found " << versionSelected << ": " << std::endl;
			m_SetupOutput << m_Description.str();

			// setup context and queue
			{
				cl_int err = CL_SUCCESS;
				m_Context = cl::Context(m_Device, NULL, NULL, NULL, &err);
				if ((m_Context() == 0) || (err != CL_SUCCESS)) {
					m_SetupOutput << "Failed to create the context. Bad installation?" << std::endl;
					if (err != CL_SUCCESS) m_SetupOutput << "   " << ClErrorToString(err) << std::endl;
					result = false;
				}
				else
				{
					m_SetupOutput << "CL Context valid." << std::endl;
				}

				m_Queue = cl::CommandQueue(m_Context, m_Device, 0, &err);
				if ((m_Queue() == 0) || (err != CL_SUCCESS)) {
					m_SetupOutput << "Failed to create the queue. Bad installation?" << std::endl;
					if (err != CL_SUCCESS) m_SetupOutput << "   " << ClErrorToString(err) << std::endl;
					result = false;
				}
				else
				{
					m_SetupOutput << "CL Queue valid." << std::endl;
				}
			}

			m_PlatformDeviceContextFound = result;

			return result;
		}

		bool Commit()
		{						
			cl::Platform p = cl::Platform::setDefault(m_Platform);
			cl::Device d = cl::Device::setDefault(m_Device);
			cl::Context c = cl::Context::setDefault(m_Context);
			cl::CommandQueue q = cl::CommandQueue::setDefault(m_Queue);

			return (p == m_Platform) && (d == m_Device) && (c == m_Context) && (q == m_Queue);
		}

		bool BuildKernels(std::string kernelString, cl::Program& resultProgram, bool retainObject = false)
		{
			m_BuildOutput.clear();						
			cl_int err = CL_SUCCESS;

			// Create kernel program using the wrapper
			resultProgram = cl::Program(kernelString, retainObject, &err);
			if (err != CL_SUCCESS)
			{
				m_BuildOutput <<"Create program failed:" << ClErrorToString(err) << std::endl;
			}
			else
			{
				// Build kernel program using the wrapper
				err = resultProgram.build("-cl-std=CL1.1");

				if (err != CL_SUCCESS)
				{
					m_BuildOutput << "Build program failed:" << ClErrorToString(err) << std::endl;
					m_BuildString = resultProgram.getBuildInfo<CL_PROGRAM_BUILD_LOG>(m_Device);
					m_BuildOutput << m_BuildString;
					m_BuildOutput << std::endl;
				}
			}

			if (err != CL_SUCCESS)
			{
				// for debugging sake, check if the build succeeds natively, and print the debugging log for the kernel
				{
					cl_device_id device_id = m_Device();
					cl_context cl_context = m_Context();
					err = CL_SUCCESS;

					// Create kernel program
					const char* table[1] = { kernelString.c_str() };
					cl_program p = clCreateProgramWithSource(cl_context, 1, (const char **)table, NULL, &err);
					if (err != CL_SUCCESS)
					{
						m_BuildOutput << "C-API Create program failed:" << ClErrorToString(err) << std::endl;
					}
					else
					{
						// Build Kernel Program
						err = clBuildProgram(p, 1, &device_id, NULL, NULL, NULL);
						if (err != CL_SUCCESS)
						{
							m_BuildOutput << "C-API Build program failed:" << ClErrorToString(err) << std::endl;
						}
					}

					size_t len;
					char *buffer;
					clGetProgramBuildInfo(p, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &len);
					buffer = (char*)malloc(len);
					clGetProgramBuildInfo(p, device_id, CL_PROGRAM_BUILD_LOG, len, buffer, NULL);
					m_BuildOutput << buffer << std::endl;
					free(buffer);
				}
				return false;
			}

			return true;
		}
	};
}

#endif
