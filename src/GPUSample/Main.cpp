#include "RendererAPI.h" // Include for renderer library
#include <map>
#include <iostream> // For printing information to the console
#include <cstring> // For comparing strings
#include <sstream> // For parsing group and filter comma separated lists
#include <iostream> // For printing information to the console
#include "CommonCL.h"
#include "ExamplesCL_C_API.h"
#include "ExamplesCL_CPP.h"
#include <stdio.h>  /* defines FILENAME_MAX */
#if defined(_WIN32)
#	include <direct.h>
#	define GetCurrentDir _getcwd
#else
#	include <unistd.h>
#	define GetCurrentDir getcwd
#endif

namespace {
	std::string GetCurrentWorkingDir(void) {
		char buff[FILENAME_MAX];
		GetCurrentDir(buff, FILENAME_MAX);
		std::string current_working_dir(buff);
		return current_working_dir;
	}
}

/****************************************************************/
/* INCLUDES FOR THE FILTERS YOU WANT TO APPLY                   */
/****************************************************************/
//#include "Filters/Desaturate_CPU.h"
//#include "Filters/Desaturate_GPU.h"
//#include "Filters/Invert_CPU.h"
//#include "Filters/Invert_GPU.h"
//#include "Filters/GS_CPU.h"
//#include "Filters/GS_GPU.h"
//#include "Filters/Noise_GPU.h"
//#include "Filters/GB_CPU.h"
//#include "Filters/GB_GPU.h"
//#include "Filters/GB_GPU_Global.h"
//#include "Filters/Sharpen_CPU.h"
//#include "Filters/Sharpen_GPU.h"
//#include "Filters/Sharpen_GPU_Global.h"
//#include "Filters/ED_CPU.h"
//#include "Filters/ED_GPU.h"
//#include "Filters/ED_GPU_Global.h"
//#include "Filters/EdgeFilterGPU.h"
//#include "Filters/Median_CPU.h"
//#include "Filters/Median_GPU.h"
#include "Filters/H_CPU.h"
#include "Filters/H_GPU.h"

/*******************************************/
/* MAIN FUNCTION (APPLICATION ENTRY POINT) */
/*******************************************/
// Image filter project main function
int main(int argc, const char* argv[]) {
	HOOK_OUTPUT;

	// This selects the 'best' device on your system.
	// It iterates across all platforms, devices and versions.
	// The highest version will be selected
	Examples::CLContext clContextANY(true, true, true); // any opencl will do
	Examples::CLContext& context = clContextANY;

	if (!context) {
		// if no device/platform could be detected, try to print the ones that are searched/found
		// for debug purpouses: list and print device and platform information, this is
		// useful if you have problems running opencl on your hardware
		Examples::CLContext clContextCPU(false, true);				// try CPU devices only
		/*if (!clContextCPU) */ std::cout << clContextCPU.GetSetupOutput() << std::endl;
		Examples::CLContext clContextGPU(true, false);				// try GPU devices only
		/*if (!clContextGPU) */ std::cout << clContextGPU.GetSetupOutput() << std::endl;
		Examples::CLContext clContextACC(false, false, true);		// try accelerators only
		/*if (!clContextACC) */ std::cout << clContextACC.GetSetupOutput() << std::endl;
	} else {
		if (context.Commit()) { std::cout << "Using:" << std::endl << clContextANY.GetDescription() << std::endl; } 
		else { std::cout << "A different context was already committed! Only one can be committed at any time." << std::endl; }
	}

	/************/
	/* EXAMPLES */
	/************/
	//Examples::RunBasicCLExample_C_API();
	//std::cout << std::endl;
	//Examples::RunBasicCLExample_CPP1(context);
	//std::cout << std::endl;
	//Examples::RunBasicCLExample_CPP2(context);
	//std::cout << std::endl;	

	/***************/
	/* ASSIGNMENTS */
	/***************/
	// Create a FilterManager object to perform image and filter operations
	Framework* const fw = Framework::GetInstance();
	FilterManager* const filterManager = FilterManager::GetInstance(); {
		// Load the image from file. You can also load your own.
		std::string currentWD = GetCurrentWorkingDir();
		std::replace(currentWD.begin(), currentWD.end(), '\\', '/');
		Image* const sourceImageLandscape = fw->LoadImageFromFile("../../resources/images/GrassLandscape.jpg");
		Image* const sourceImageLena = fw->LoadImageFromFile("../../resources/images/Lena.png");
		Image* const sourceImagesalty = fw->LoadImageFromFile("../../resources/images/LenaSaltPepperNoise.gif");
		Image* const sourceImageGS = fw->LoadImageFromFile("../../resources/images/GreenScreen.jpg");
		//Image* const sourceImageGray = fw->LoadImageFromFile("../../resources/images/HawkesBay.jpg");
		Image* const sourceImageGray = fw->LoadImageFromFile("../../resources/images/HawkesBayGray.jpg");

		/***********/
		/* FILTERS */
		/***********/
		if (sourceImageLandscape && sourceImageLena && sourceImageGS && sourceImagesalty && sourceImageGray) {
			// register filters into filter manager
			/*filterManager->RegisterFilter("Desaturate", new Filters::Desaturate_CPU(), new Filters::Desaturate_GPU(context));
			filterManager->RegisterFilter("Invert", new Filters::Invert_CPU(), new Filters::Invert_GPU(context));
			filterManager->RegisterFilter("GS", new Filters::GS_CPU(), new Filters::GS_GPU(context));
			filterManager->RegisterFilter("Noise", NULL, new Filters::Noise_GPU(context));
			filterManager->RegisterFilter("GB", new Filters::GB_CPU(), new Filters::GB_GPU(context));
			filterManager->RegisterFilter("GB_Global", new Filters::GB_CPU(), new Filters::GB_GPU_Global(context));
			filterManager->RegisterFilter("Sharpen", new Filters::Sharpen_CPU(), new Filters::Sharpen_GPU(context));
			filterManager->RegisterFilter("ED", new Filters::ED_CPU(), new Filters::ED_GPU(context));
			filterManager->RegisterFilter("Median", new Filters::Median_CPU(), new Filters::Median_GPU(context));
			filterManager->RegisterFilter("Sharpen", new Filters::Sharpen_CPU(), new Filters::Sharpen_GPU(context));*/
			filterManager->RegisterFilter("Histogram", new Filters::H_CPU(), new Filters::H_GPU(context));

			// execute filters on the input images
			/*filterManager->ExecuteFilter("Desaturate", sourceImageLandscape);
			filterManager->ExecuteFilter("Invert", sourceImageLandscape);
			filterManager->ExecuteFilter("GS", sourceImageGS);
			filterManager->ExecuteFilter("Noise", sourceImageLandscape);
			filterManager->ExecuteFilter("GB", sourceImageLandscape);
			filterManager->ExecuteFilter("Sharpen", sourceImageLandscape);
			filterManager->ExecuteFilter("ED", sourceImageLandscape);
			filterManager->ExecuteFilter("Median", sourceImagesalty);
			filterManager->ExecuteFilter("Sharpen", sourceImageLandscape);*/
			filterManager->ExecuteFilter("Histogram", sourceImageGray);
		}
		std::cout << std::endl << "Filter report:" << std::endl;
		std::cout << filterManager->GetOutput();
		/***************/
		/* CLEANUP !!! */
		/***************/
		delete sourceImageLandscape;
		delete sourceImageLena;
		delete sourceImagesalty;
		delete sourceImageGS;
		delete sourceImageGray;
	}
	delete filterManager;
	delete fw;
	///////////////////////
	// EXTRA SAMPLE CODE //
	///////////////////////
	/*
	// Preview individual images
	fw->DisplaySingleImage(sourceImage);
	// Preview a group of images
	std::vector<Image*> images;
	images.push_back(sourceImage);
	images.push_back(destinationImageDesaturate);
	fw->DisplayMultipleImages(images);
	// Preview a comparison of two images
	fw->DisplayImageComparison(sourceImage, destinationImageDesaturate);
	// Delete the image from memory
	delete sourceImage;
	delete destinationImageDesaturate;
	delete destinationImageBlur;
	*/
	UNHOOK_OUTPUT;
}