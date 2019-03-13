#ifndef H_OPENCL
#define H_OPENCL


#include <CL/cl.h>
#include <CL/cl_platform.h>
#include <CL/cl_ext.h>
#include <CL/cl_gl.h>
#include <CL/cl_gl_ext.h>

//#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_MINIMUM_OPENCL_VERSION 110
#define CL_HPP_TARGET_OPENCL_VERSION 110
#define CL_HPP_ENABLE_SIZE_T_COMPATIBILITY

#include "CL/cl2.hpp" // local instance of cl2.hpp
#include <memory>
#include <algorithm>

#include <iostream> // For printing information to the console
#include <fstream> // For reading shader files

extern std::string ClErrorToString(cl_int err);
#endif