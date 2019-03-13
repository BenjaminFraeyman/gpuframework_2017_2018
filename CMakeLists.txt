cmake_minimum_required( VERSION 2.8 )
################################################################
# Generate the project and set its dependencies                #
################################################################

# Project name
project( "GPUProgramming" )

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/Modules" ${CMAKE_MODULE_PATH})	# To make sure we use the same cmake-modules provided by us first

set(CMAKE_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/include")	# The included (3rd party) header files WE want to use 	(only if not in here, search system)
set(CMAKE_LIBRARY_PATH "${CMAKE_SOURCE_DIR}/lib")		# The (3rd party) library files WE want to use			(only if not in here, search system)

set(CMAKE_CXX_STANDARD 11) # C++11...
set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...
set(CMAKE_CXX_EXTENSIONS OFF) #...without compiler extensions like gnu++11

# Determine 64bit vs 32 bit
if (WIN32)
    IF(CMAKE_EXE_LINKER_FLAGS MATCHES "x64")
        set(PROJECT_ARCHITECTURE "x64")
    ELSE()
        set(PROJECT_ARCHITECTURE "x86")
    ENDIF()
else() # UNIX
    IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(PROJECT_ARCHITECTURE "x64")
    ELSE()
        set(PROJECT_ARCHITECTURE "x86")
    ENDIF()
endif()

SET(USE_COMPUTE_FRAMEWORK "OpenCL" CACHE STRING "Compute framework selected at CMake configure time")
SET_PROPERTY(CACHE USE_COMPUTE_FRAMEWORK PROPERTY STRINGS OpenCL AMP) 
# IF(NOT(DEFINED COMPUTE_FRAMEWORK))

     # SET(COMPUTE_FRAMEWORK "NotAnOption" CACHE STRING "Last loaded compute framework")

     # MARK_AS_ADVANCED (FORCE COMPUTE_FRAMEWORK)

# ENDIF()

# IF(NOT (${USE_COMPUTE_FRAMEWORK} MATCHES ${COMPUTE_FRAMEWORK}))

     # UNSET(COMPUTE_FRAMEWORK CACHE)

     # SET(COMPUTE_FRAMEWORK ${USE_COMPUTE_FRAMEWORK} CACHE STRING "Updating Library Project Configuration Option" FORCE)

# ENDIF()

message(STATUS "Using x64 DLL folder for dynamic linked libraries")
set (DLL_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/dll/vs2017/x64)     
#set (LIB_FOLDER ${CMAKE_LIBRARY_PATH}/x64)

add_subdirectory(src)
