#
# Try to find GLEW library and include path.
# Once done this will define
#
# GLEW_FOUND
# GLEW_INCLUDE_PATH
# GLEW_LIBRARY
# 
SET (GLEW_USE_MULTITHREADING OFF CACHE BOOL "Use Multithreaded version of Glew")
IF (WIN32)
	FIND_PATH( GLEW_INCLUDE_PATH GL/glew.h
		${CMAKE_INCLUDE_PATH}
		$ENV{GLEW_ROOT_DIR}/include
		DOC "The directory where GL/glew.h resides")

	IF (CMAKE_CL_64)
		IF (GLEW_USE_MULTITHREADING)
			FIND_LIBRARY( GLEW_LIBRARY
				NAMES glew32mx glew32mxs
				PATHS			
				"${LIB_FOLDER}"
				"$ENV{GLEW_ROOT_DIR}"
				PATH_SUFFIXES
				"lib/x64" "lib" "x64" "Release MX" "lib/Release MX/x64"
				DOC "The GLEW library (64-bit)"
				SUFFIX ".lib"
			)
			FIND_FILE( GLEW_DLL 
			NAMES glew32mx.dll glew32mxs.dll
				PATHS
				"${DLL_FOLDER}"
				"$ENV{GLEW_ROOT_DIR}"
				PATH_SUFFIXES 
				"bin/x64" "bin" "x64" "Release MX" "bin/Release MX/x64"
				DOC "The GLEW dll")
		ELSE()
		FIND_LIBRARY( GLEW_LIBRARY
			NAMES glew GLEW glew32 glew32s
			PATHS			
			"${LIB_FOLDER}"
			"$ENV{GLEW_ROOT_DIR}"
			PATH_SUFFIXES
			"lib/x64" "lib" "x64" "Release" "lib/Release/x64"
			DOC "The GLEW library (64-bit)"
			SUFFIX ".lib"
		)
		# SET(TEMP ${CMAKE_FIND_LIBRARY_SUFFIXES})
		#SET(CMAKE_FIND_LIBRARY_SUFFIXES "")
		FIND_FILE( GLEW_DLL 
		NAMES glew.dll glew32.dll glew32s.dll
			PATHS
			"${DLL_FOLDER}"
			"$ENV{GLEW_ROOT_DIR}"
			PATH_SUFFIXES 
			"bin/x64" "bin" "x64" "Release" "bin/Release/x64"
			DOC "The GLEW dll")
		# FIND_LIBRARY( GLEW_DLL
			# NAMES glew GLEW glew32 glew32s
			# PATHS
			# "${LIB_FOLDER}"
			# "$ENV{GLEW_ROOT_DIR}"
			# PATH_SUFFIXES 
			# "bni/x64" "lib" "x64" "Release" "bin/Release/x64"
			# SUFFIX
			# ".dll"
			# DOC "The GLEW dll"
		# )
		# SET(CMAKE_FIND_LIBRARY_SUFFIXES TEMP)
		# UNSET(TEMP)
		ENDIF()
	ELSE() #32 bit
		IF (GLEW_USE_MULTITHREADING)
			FIND_LIBRARY( GLEW_LIBRARY
				NAMES glew32mx glew32mxs
				PATHS
				"${LIB_FOLDER}"
				"$ENV{GLEW_ROOT_DIR}"
				PATH_SUFFIXES 
				"lib/x86" "lib" "x86" "lib/Win32" "Win32" "Release MX" "lib/Release MX/Win32"
				DOC "The GLEW library"
			)
			FIND_FILE( GLEW_DLL 
				NAMES glew32mx.dll glew32mxs.dll
				PATHS
				"${DLL_FOLDER}"
				"$ENV{GLEW_ROOT_DIR}"
				PATH_SUFFIXES 
				"bin/x86" "bin" "x86" "bin/Win32" "Win32" "Release MX" "bin/Release MX/Win32"
				DOC "The GLEW dll")
		ELSE()
		FIND_LIBRARY( GLEW_LIBRARY
			NAMES glew GLEW glew32 glew32s
			PATHS
			"${LIB_FOLDER}"
			"$ENV{GLEW_ROOT_DIR}"
			PATH_SUFFIXES 
			"lib/x86" "lib" "x86" "lib/Win32" "Win32" "Release" "lib/Release/Win32"
			DOC "The GLEW library"
		)
		# SET(TEMP ${CMAKE_FIND_LIBRARY_SUFFIXES})
		# SET(CMAKE_FIND_LIBRARY_SUFFIXES ".dll")
		# FIND_LIBRARY( GLEW_DLL
			# NAMES glew GLEW glew32 glew32s
			# PATHS
			# "${LIB_FOLDER}"
			# "$ENV{GLEW_ROOT_DIR}"
			# PATH_SUFFIXES 
			# "lib/x86" "lib" "x86"
			# SUFFIX
			# ".dll"
			# DOC "The GLEW dll"
		# )
		# SET(CMAKE_FIND_LIBRARY_SUFFIXES TEMP)
		# UNSET(TEMP)
		FIND_FILE( GLEW_DLL 
			NAMES glew.dll glew32.dll glew32s.dll
			PATHS
			"${DLL_FOLDER}"
			"$ENV{GLEW_ROOT_DIR}"
			PATH_SUFFIXES 
			"bin/x86" "bin" "x86" "bin/Win32" "Win32" "Release" "bin/Release/Win32"
			DOC "The GLEW bin")
		ENDIF()
	ENDIF()
ELSE (WIN32)
	FIND_PATH( GLEW_INCLUDE_PATH GL/glew.h
		/usr/include
		/usr/local/include
		/sw/include
		/opt/local/include
		${GLEW_ROOT_DIR}/include
		DOC "The directory where GL/glew.h resides")
	IF (GLEW_USE_MULTITHREADING)
		FIND_LIBRARY( GLEW_LIBRARY
		NAMES GLEWmx glewmx
		PATHS
		/usr/lib64
		/usr/lib
		/usr/local/lib64
		/usr/local/lib
		/sw/lib
		/opt/local/lib
		${GLEW_ROOT_DIR}/lib
		DOC "The GLEW library")
	ELSE()
		FIND_LIBRARY( GLEW_LIBRARY
		NAMES GLEW glew
		PATHS
		/usr/lib64
		/usr/lib
		/usr/local/lib64
		/usr/local/lib
		/sw/lib
		/opt/local/lib
		${GLEW_ROOT_DIR}/lib
		DOC "The GLEW library")
	ENDIF()
ENDIF (WIN32)



include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLEW "Glew could not be found, did you set GLEW_ROOT_DIR environment variable?" GLEW_LIBRARY GLEW_INCLUDE_PATH)

if(GLEW_FOUND)
    SET(GLEW_LIBRARIES ${GLEW_LIBRARY})
    include_directories(SYSTEM ${GLEW_INCLUDE_PATH})
endif()
