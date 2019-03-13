#
# Try to find the FreeImage library and include path.
# Once done this will define
#
# FREEIMAGE_FOUND
# FREEIMAGE_INCLUDE_PATH
# FREEIMAGE_LIBRARY
#

IF (WIN32)

	FIND_PATH( FREEIMAGE_INCLUDE_PATH FreeImage.h
				${CMAKE_INCLUDE_PATH}
				$ENV{FREEIMAGE_DIR}/include/FreeImage
				DOC "The directory where FreeImage.h resides")
	
	IF (CMAKE_CL_64)
		FIND_LIBRARY( FREEIMAGE_LIBRARY
				NAMES FreeImage
				PATHS 
				"${LIB_FOLDER}"
				$ENV{FREEIMAGE_DIR}/lib/x64
				DOC "The FreeImage library"
		)
		
		FIND_LIBRARY( FREEIMAGE_LIBRARY_D
				NAMES FreeImageD
				PATHS 
				"${LIB_FOLDER}"
				$ENV{FREEIMAGE_DIR}/lib/x64
				DOC "The FreeImage library"
		)
		
		FIND_FILE( FREEIMAGE_DLL 
			NAMES FreeImage.dll
			PATHS
			"${DLL_FOLDER}"
			$ENV{FREEIMAGE_DIR}/dll
			PATH_SUFFIXES 
			"dll/x64" "dll" "x64"
			DOC "The FREEIMAGE dll")
			
		FIND_FILE( FREEIMAGE_DLL_D
			NAMES FreeImageD.dll
			PATHS
			"${DLL_FOLDER}"
			$ENV{FREEIMAGE_DIR}/dll
			PATH_SUFFIXES 
			"dll/x64" "dll" "x64"
			DOC "The FREEIMAGE dll")
	else()
		FIND_LIBRARY( FREEIMAGE_LIBRARY
				NAMES FreeImage
				PATHS 
				"${LIB_FOLDER}"
				$ENV{FREEIMAGE_DIR}/lib/x86
				DOC "The FreeImage library"
		)
		FIND_LIBRARY( FREEIMAGE_LIBRARY_D
				NAMES FreeImage	# using release lib here (for some reason this was not available?)
				PATHS 
				"${LIB_FOLDER}"
				$ENV{FREEIMAGE_DIR}/lib/x86
				DOC "The FreeImage library"
		)
		
		FIND_FILE( FREEIMAGE_DLL 
			NAMES FreeImage.dll
			PATHS
			"${DLL_FOLDER}"
			$ENV{FREEIMAGE_DIR}/dll
			PATH_SUFFIXES 
			"dll/x86" "dll" "x86"
			DOC "The FREEIMAGE dll")
		FIND_FILE( FREEIMAGE_DLL_D
			NAMES FreeImageD.dll
			PATHS
			"${DLL_FOLDER}"
			$ENV{FREEIMAGE_DIR}/dll
			PATH_SUFFIXES 
			"dll/x86" "dll" "x86"
			DOC "The FREEIMAGE dll")
	endif()	
			
ELSE()

	FIND_PATH( FREEIMAGE_INCLUDE_PATH FreeImage.h
				${CMAKE_INCLUDE_PATH}/FreeImage
				$ENV{FREEIMAGE_DIR}/include
				/usr/include
				/usr/local/include
				/sw/include
				/opt/local/include
				DOC "The directory where FreeImage.h resides")
	
	FIND_LIBRARY( FREEIMAGE_LIBRARY
				NAMES FreeImage freeimage
				PATHS
				"${LIB_FOLDER}"
				/usr/lib64
				/usr/lib
				/usr/local/lib64
				/usr/local/lib
				/sw/lib
				/opt/local/lib
				$ENV{FREEIMAGE_DIR}/lib
				DOC "The FreeImage library")
				
	FIND_FILE( FREEIMAGE_DLL 
			NAMES FreeImage.so
			PATHS
			"${DLL_FOLDER}"
			$ENV{FREEIMAGE_DIR}/dll
			PATH_SUFFIXES 
			"dll/x86" "dll" "x86"
			DOC "The FREEIMAGE dll")
	
ENDIF()

SET(FREEIMAGE_LIBRARIES ${FREEIMAGE_LIBRARY} )
SET(FREEIMAGE_DLL ${FREEIMAGE_DLL} ${FREEIMAGE_DLL_D})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FREEIMAGE "FreeImage could not be found, did you set FREEIMAGE_DIR environment variable?" FREEIMAGE_LIBRARY FREEIMAGE_LIBRARIES FREEIMAGE_INCLUDE_PATH)

if(FREEIMAGE_FOUND)
    include_directories(SYSTEM ${FREEIMAGE_INCLUDE_PATH})
endif()
