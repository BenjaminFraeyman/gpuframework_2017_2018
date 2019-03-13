cmake_minimum_required( VERSION 2.8 )

# Search for dependent libraries
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OpenCL REQUIRED)
find_package(GLFW REQUIRED)
find_package(FreeImage REQUIRED)

if(GLFW_FOUND AND FREEIMAGE_FOUND AND OPENCL_FOUND)
    # We can specify which directories we want to search through when including headers
    # (See Visual Studio's "Additional Include Directories", this gets filled in using this command)
    # TODO: this seems to be the complete path, so while this will work for us to develop, we cannot simply distribute these projects as such!
    include_directories( ../../include )
    include_directories(${OpenCL_INCLUDE_DIR})
    include_directories(${OPENCL_INCLUDE_DIRS})
    include_directories(${GLFW_INCLUDE_DIR})


    ################################################################
    # Structure the source and header files                        #
    ################################################################

    # set: just set the variables "srcfiles" and "libfiles"
    set( SRC_LIBPROJECT
        "Framework.h"
        "Framework.cpp"
        "FilterManager.h"
        "FilterManager.cpp"
        "Image.h"
        "Image.cpp"
        "EnumPreviewType.h"
        "OpenCL.h"
        "StructImagePreviewState.h"
        "ImageFilter.h"
        )
    set(GPUPROG_RENDERER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include  CACHE INTERNAL "")
    set(GPUPROG_RENDERER_INCLUDE_DIR ${GLEW_INCLUDE_PATH} ${GPUPROG_RENDERER_INCLUDE_DIR}  CACHE INTERNAL "")
    set(GPUPROG_RENDERER_INCLUDE_DIR ${OpenCL_INCLUDE_DIR} ${GPUPROG_RENDERER_INCLUDE_DIR}  CACHE INTERNAL "")
    set(GPUPROG_RENDERER_INCLUDE_DIR ${OPENCL_INCLUDE_DIRS} ${GPUPROG_RENDERER_INCLUDE_DIR}  CACHE INTERNAL "")
    set(GPUPROG_RENDERER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../include ${GPUPROG_RENDERER_INCLUDE_DIR}  CACHE INTERNAL "")
        
    SOURCE_GROUP(Renderer FILES ${SRC_LIBPROJECT})
    add_library(Renderer ${SRC_LIBPROJECT})
    
    set(GPUPROG_RENDERER Renderer ${GLFW_LIBRARIES} ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${FREEIMAGE_LIBRARIES} ${OpenCL_LIBRARY} ${OPENCL_LIBRARIES} CACHE INTERNAL "")

ELSE()
    message("Dependent library(ies) missing: GLFW ${GLFW_FOUND} FREEIMAGE ${FREEIMAGE_FOUND} OPENCL ${OPENCL_FOUND}")
ENDIF()