cmake_minimum_required( VERSION 2.8 )

################################################################
# Structure the source and header files                        #
################################################################

# set: just set the variables "srcfiles" and "libfiles"
set( SRC_FRAMEWORK
	"Main.cpp"
	"ExamplesCL_C_API.cpp"
	"ExamplesCL_C_API.h"
	"ExamplesCL_CPP.cpp"
	"ExamplesCL_CPP.h"
	"CommonCL.h"
	)

set( SRC_FILTERS
	"Filters/Desaturate_CPU.h"
	"Filters/Desaturate_CPU.cpp"
	"Filters/Desaturate_GPU.h"
	"Filters/Desaturate_GPU.cpp"
	"Filters/GS_CPU.h"
	"Filters/GS_CPU.cpp"
	"Filters/GS_GPU.h"
	"Filters/GS_GPU.cpp"
	"Filters/Invert_CPU.h"
	"Filters/Invert_CPU.cpp"
	"Filters/Invert_GPU.h"
	"Filters/Invert_GPU.cpp"
	"Filters/GB_CPU.h"
	"Filters/GB_CPU.cpp"
	"Filters/GB_GPU.h"
	"Filters/GB_GPU.cpp"
	"Filters/GB_GPU_Global.h"
	"Filters/GB_GPU_Global.cpp"
	"Filters/Sharpen_CPU.h"
	"Filters/Sharpen_CPU.cpp"
	"Filters/Sharpen_GPU.h"
	"Filters/Sharpen_GPU.cpp"
	"Filters/Sharpen_GPU_Global.h"
	"Filters/Sharpen_GPU_Global.cpp"
	"Filters/ED_CPU.h"
	"Filters/ED_CPU.cpp"
	"Filters/ED_GPU.h"
	"Filters/ED_GPU.cpp"
	"Filters/EdgeFilterGPU.h"
	"Filters/EdgeFilterGPU.cpp"
	"Filters/ED_GPU_Global.h"
	"Filters/ED_GPU_Global.cpp"
	"Filters/Median_CPU.h"
	"Filters/Median_CPU.cpp"
	"Filters/Median_GPU.h"
	"Filters/Median_GPU.cpp"
	"Filters/H_CPU.h"
	"Filters/H_CPU.cpp"
	"Filters/H_GPU.h"
	"Filters/H_GPU.cpp"
	"Filters/Hist.cl"
	"Filters/equalization.cl"
	"Filters/Noise_GPU.h"
	"Filters/Noise_GPU.cpp"
	)

SOURCE_GROUP(GPGPU FILES ${SRC_FRAMEWORK}) # Creates source filter structure for side-pane in VS
SOURCE_GROUP(GPGPU\\Filters FILES ${SRC_FILTERS})

include_directories(${GPUPROG_RENDERER_INCLUDE_DIR})
# create the target (aka, project, which we define here to be an "executable") "GPUSample"
add_executable( GPUSample ${SRC_FRAMEWORK} ${SRC_FILTERS})
# do correct linking
target_link_libraries( GPUSample ${GPUPROG_RENDERER} )

################################################################
# Copy the DLL files to the Build directory                    #
################################################################

# Source folder for the DLL files

IF(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	IF(NOT ${GENERATOR_FOLDER} MATCHES "null")
		# Copy DLL files over to the build directory
		add_custom_command(TARGET GPUSample POST_BUILD 
			COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DLL_FOLDER}/glew32.dll" $<TARGET_FILE_DIR:GPUSample> 
			COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DLL_FOLDER}/FreeImage.dll" $<TARGET_FILE_DIR:GPUSample>  
		)
	ENDIF()
ENDIF()