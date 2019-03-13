#include "Framework.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "OpenCL.h"
#include "Image.h"
#include "ImageFilter.h"

#include "FreeImage.h" // For loading various image file formats

#include <iostream> // For printing information to the console
#include <fstream> // For reading shader files
#include <ctime> // For mantaining a stable framerate in the update window
#include <vector> // For passing shader compilation errors

#if (_MSC_VER < 1800)	// _MSC_VER == 1800 -> MSVC++ 12.0 or Visual Studio 2013 (http://stackoverflow.com/questions/70013/how-to-detect-if-im-compiling-code-with-visual-studio-2008)
#include <algorithm>
#define roundf(X) floorf(X + 0.5f)
#define fminf(X,Y) std::min<float>(X,Y)
#define fmaxf(X,Y) std::max<float>(X,Y)
#endif


std::string ClErrorToString(cl_int err)
{
	switch (err) {
	case CL_SUCCESS:                            return "Success!";											
	case CL_DEVICE_NOT_FOUND:                   return "Device not found.";
	case CL_DEVICE_NOT_AVAILABLE:               return "Device not available";
	case CL_COMPILER_NOT_AVAILABLE:             return "Compiler not available";
	case CL_MEM_OBJECT_ALLOCATION_FAILURE:      return "Memory object allocation failure";
	case CL_OUT_OF_RESOURCES:                   return "Out of resources";
	case CL_OUT_OF_HOST_MEMORY:                 return "Out of host memory";
	case CL_PROFILING_INFO_NOT_AVAILABLE:       return "Profiling information not available";
	case CL_MEM_COPY_OVERLAP:                   return "Memory copy overlap";
	case CL_IMAGE_FORMAT_MISMATCH:              return "Image format mismatch";
	case CL_IMAGE_FORMAT_NOT_SUPPORTED:         return "Image format not supported";
	case CL_BUILD_PROGRAM_FAILURE:              return "Program build failure";
	case CL_MAP_FAILURE:                        return "Map failure";
	case CL_INVALID_VALUE:                      return "Invalid value";
	case CL_INVALID_DEVICE_TYPE:                return "Invalid device type";
	case CL_INVALID_PLATFORM:                   return "Invalid platform";
	case CL_INVALID_DEVICE:                     return "Invalid device";
	case CL_INVALID_CONTEXT:                    return "Invalid context";
	case CL_INVALID_QUEUE_PROPERTIES:           return "Invalid queue properties";
	case CL_INVALID_COMMAND_QUEUE:              return "Invalid command queue";
	case CL_INVALID_HOST_PTR:                   return "Invalid host pointer";
	case CL_INVALID_MEM_OBJECT:                 return "Invalid memory object";
	case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:    return "Invalid image format descriptor";
	case CL_INVALID_IMAGE_SIZE:                 return "Invalid image size";
	case CL_INVALID_SAMPLER:                    return "Invalid sampler";
	case CL_INVALID_BINARY:                     return "Invalid binary";
	case CL_INVALID_BUILD_OPTIONS:              return "Invalid build options";
	case CL_INVALID_PROGRAM:                    return "Invalid program";
	case CL_INVALID_PROGRAM_EXECUTABLE:         return "Invalid program executable";
	case CL_INVALID_KERNEL_NAME:                return "Invalid kernel name";
	case CL_INVALID_KERNEL_DEFINITION:          return "Invalid kernel definition";
	case CL_INVALID_KERNEL:                     return "Invalid kernel";
	case CL_INVALID_ARG_INDEX:                  return "Invalid argument index";
	case CL_INVALID_ARG_VALUE:                  return "Invalid argument value";
	case CL_INVALID_ARG_SIZE:                   return "Invalid argument size";
	case CL_INVALID_KERNEL_ARGS:                return "Invalid kernel arguments";
	case CL_INVALID_WORK_DIMENSION:             return "Invalid work dimension";
	case CL_INVALID_WORK_GROUP_SIZE:            return "Invalid work group size";
	case CL_INVALID_WORK_ITEM_SIZE:             return "Invalid work item size";
	case CL_INVALID_GLOBAL_OFFSET:              return "Invalid global offset";
	case CL_INVALID_EVENT_WAIT_LIST:            return "Invalid event wait list";
	case CL_INVALID_EVENT:                      return "Invalid event";
	case CL_INVALID_OPERATION:                  return "Invalid operation";
	case CL_INVALID_GL_OBJECT:                  return "Invalid OpenGL object";
	case CL_INVALID_BUFFER_SIZE:                return "Invalid buffer size";
	case CL_INVALID_MIP_LEVEL:                  return "Invalid mip - map level";

	case CL_MISALIGNED_SUB_BUFFER_OFFSET:				return "CL_MISALIGNED_SUB_BUFFER_OFFSET				";
	case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:  return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
	case CL_INVALID_GLOBAL_WORK_SIZE:					return "CL_INVALID_GLOBAL_WORK_SIZE					";
	case CL_INVALID_PROPERTY:							return "CL_INVALID_PROPERTY							";

#if CL_HPP_TARGET_OPENCL_VERSION == 120 || CL_HPP_TARGET_OPENCL_VERSION == 200
	case CL_COMPILE_PROGRAM_FAILURE:					return "CL_COMPILE_PROGRAM_FAILURE					";
	case CL_LINKER_NOT_AVAILABLE:						return "CL_LINKER_NOT_AVAILABLE						";
	case CL_LINK_PROGRAM_FAILURE:						return "CL_LINK_PROGRAM_FAILURE						";
	case CL_DEVICE_PARTITION_FAILED:					return "CL_DEVICE_PARTITION_FAILED					";
	case CL_KERNEL_ARG_INFO_NOT_AVAILABLE:				return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE			";
	case CL_INVALID_IMAGE_DESCRIPTOR:					return "CL_INVALID_IMAGE_DESCRIPTOR					";
	case CL_INVALID_COMPILER_OPTIONS:					return "CL_INVALID_COMPILER_OPTIONS					";
	case CL_INVALID_LINKER_OPTIONS:						return "CL_INVALID_LINKER_OPTIONS					";
	case CL_INVALID_DEVICE_PARTITION_COUNT:				return "CL_INVALID_DEVICE_PARTITION_COUNT			";
#endif
	default: return "Unknown";
	}
}

/****************************************************************/
/* SINGLETON					                                */
/****************************************************************/

Framework* Framework::instance = NULL;

Framework* Framework::GetInstance()
{
	if (!instance){
		instance = new Framework();
	}
	return instance;
}



/****************************************************************/
/* INITIALIZATION AND TERMINATION                               */
/****************************************************************/

bool Framework::Initialize()
{
	// Settings and parameters
	previewWindowAALevel = 4;
	previewWindowWidth = 1280;
	previewWindowHeight = 720;
	previewWindowUpdateRate = 60;

	vertexShaderFile = "../ImageFilters/shaders/image2D.vert";
	fragmentShaderFile = "../ImageFilters/shaders/image2D.frag";

	scale = 1.0f;
	mouseXPrevious = 0.0f;
	mouseYPrevious = 0.0f;
	selectedPreviewImage = -1;
	snapgridIntervalX = 32;
	snapgridIntervalY = 32;
	comparisonPercentage = 0.5f;

	// Module initialization
	std::cout << "[Framework] Initializing... " << std::endl;

	InitializeGLFW();
	InitializeGLEW();
	InitializeFreeImage();

	InitializeShaderProgram();
	InitializeBuffers();
	InitializeInputCallbacks();

	InitializeOpenCL();

	return true;
}

bool Framework::InitializeGLFW()
{
	// Initialize GLFW
	if (!glfwInit()){
		return false;
	}

	// Set window settings and create a window
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_SAMPLES, previewWindowAALevel);
	glfwWindowHint(GLFW_REFRESH_RATE, previewWindowUpdateRate);
	previewWindow = glfwCreateWindow(previewWindowWidth, previewWindowHeight, "ImageFilter Preview", NULL, NULL);

	if (!previewWindow){
		std::cout << "[Framework :: GLFW] There was a problem creating the preview window through GLFW. " << std::endl;
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(previewWindow);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return true;
}

bool Framework::InitializeGLEW()
{
	// Initialize GLEW
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();

	if (err != GLEW_OK){
		std::cout << "[Framework :: GLEW] There was a problem initializing GLEW. " << std::endl;
		return false;
	}

	return true;
}

bool Framework::InitializeFreeImage()
{
	// Initialize FreeImage
	FreeImage_Initialise();

	return true;
}

bool Framework::InitializeOpenCL()
{
	return true;
}

bool Framework::TerminateOpenCL()
{
	return true;
}

bool Framework::InitializeShaderProgram()
{
	// Reserve space for the shader program and individual shaders
	shaderProgram = glCreateProgram();
	GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Load the shaders from file
	// LoadAndCompileShader(vertShader, vertexShaderFile);
	// LoadAndCompileShader(fragShader, fragmentShaderFile);

	// Load default vertex and fragment shaders
	LoadAndCompileShader_DefaultVertexShader(vertShader);
	LoadAndCompileShader_DefaultFragmentShader(fragShader);

	// Create the shader program
	glAttachShader(shaderProgram, vertShader);
	glAttachShader(shaderProgram, fragShader);
	glLinkProgram(shaderProgram);

	// Delete the individual shaders
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	// Activate the shader program
	glUseProgram(shaderProgram);

	return true;
}

void Framework::LoadAndCompileShader(GLuint shaderID, std::string fileShader)
{
	// Load the shader code
	std::string shaderCode;
	std::ifstream shaderCodeStream(fileShader);
	if (shaderCodeStream.is_open()){
		std::string line = "";
		while (getline(shaderCodeStream, line)){
			shaderCode += "\n" + line;
		}
		shaderCodeStream.close();
	}

	// Compile the shader
	char const * shaderCodePointer = shaderCode.c_str();
	glShaderSource(shaderID, 1, &shaderCodePointer, NULL);
	glCompileShader(shaderID);

	// Check for errors
	LoadAndCompileShader_PrintStatus(shaderID);

}

void Framework::LoadAndCompileShader_DefaultVertexShader(GLuint shaderID)
{
	std::string vertexShaderCode =
		"#version 110 \n"
		"attribute vec4 position_Modelspace; \n"
		"varying vec2 fragmentUVs; \n"
		"uniform mat4 matMVP; \n"
		"void main(void) \n"
		"{ \n"
		"	gl_Position = vec4(position_Modelspace.xy, 0.0, 1.0); \n"
		"	fragmentUVs = position_Modelspace.zw; \n"
		"} \n"
		;

	// Compile the shader
	char const * shaderCodePointer = vertexShaderCode.c_str();
	glShaderSource(shaderID, 1, &shaderCodePointer, NULL);
	glCompileShader(shaderID);

	// Check for errors
	LoadAndCompileShader_PrintStatus(shaderID);
}

void Framework::LoadAndCompileShader_DefaultFragmentShader(GLuint shaderID)
{
	std::string fragmentShaderCode =
		"#version 110 \n"
		"varying vec2 fragmentUVs; \n"
		"uniform vec4 textColor; \n"
		"uniform sampler2D texDiffuse; \n"
		"void main(void) \n"
		"{ \n"
		"gl_FragColor = texture2D(texDiffuse, fragmentUVs); \n"
		"} \n"
		;

	// Compile the shader
	char const * shaderCodePointer = fragmentShaderCode.c_str();
	glShaderSource(shaderID, 1, &shaderCodePointer, NULL);
	glCompileShader(shaderID);

	// Check for errors
	LoadAndCompileShader_PrintStatus(shaderID);
}

void Framework::LoadAndCompileShader_PrintStatus(GLuint shaderID)
{
	// Check for errors
	int infoLogLength;
	glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
	std::vector<char> shaderErrorMessage(infoLogLength);
	if (infoLogLength)
	{
		glGetShaderInfoLog(shaderID, infoLogLength, NULL, &shaderErrorMessage[0]);
		std::string shaderErrorMessageString = std::string(shaderErrorMessage.begin(), shaderErrorMessage.end());
		std::cout << "[Framework :: ShaderCompilation] Compiling shader, message (OK if empty): " << shaderErrorMessageString << std::endl;
	}
	else
	{
		std::cout << "[Framework :: ShaderCompilation] Compiling shader OK" << std::endl;
	}
}

bool Framework::InitializeBuffers()
{
	// Vertex buffer for the quads to render images
	glGenBuffers(1, &quadBuffer);

	return true;
}

bool Framework::InitializeInputCallbacks()
{
	glfwSetKeyCallback(previewWindow, GLFWKeyboardEventCallback); // Keyboard callback
	glfwSetMouseButtonCallback(previewWindow, GLFWMouseButtonEventCallback); // Mouse button callback
	glfwSetCursorPosCallback(previewWindow, GLFWMousePositionEventCallback); // Mouse position callback
	glfwSetScrollCallback(previewWindow, GLFWMouseScrollEventCallback); // Mouse scroll callback

	return true;
}

bool Framework::Terminate()
{
	std::cout << "[Framework] Terminating... " << std::endl;

	TerminateOpenCL();

	TerminateGLFW();
	TerminateGLEW();
	TerminateFreeImage();

	TerminateShaderProgram();
	TerminateBuffers();

	return true;
}

bool Framework::TerminateGLFW()
{
	// Destroy the window and terminate GLFW
	glfwDestroyWindow(previewWindow);
	glfwTerminate();

	return true;
}

bool Framework::TerminateGLEW()
{
	// Nothing
	return true;
}

bool Framework::TerminateFreeImage()
{
	// Terminate FreeImage
	FreeImage_DeInitialise();

	return true;
}

bool Framework::TerminateShaderProgram()
{
	glDeleteProgram(shaderProgram);

	return true;
}

bool Framework::TerminateBuffers()
{
	// Vertex buffer for the quads to render images
	glDeleteBuffers(1, &quadBuffer);

	return true;
}



/****************************************************************/
/* IMAGE MANAGEMENT                                             */
/****************************************************************/

void Framework::DisplayImageLoop(PreviewType previewType)
{
	previewWindowRunning = true;

	activePreviewType = previewType; // Set the active preview type, so that event callbacks know how to interpret input

	time_t tPrevious = clock();
	time_t tLag = 0;
	while (previewWindowRunning){
		time_t tCurrent = clock();
		time_t tElapsed = tCurrent - tPrevious;
		tPrevious = tCurrent;
		tLag += tElapsed;

		while (tLag >= (previewWindowUpdateRate)){
			DisplayImageLoop_Update(previewType);
			tLag -= previewWindowUpdateRate;
		}

		DisplayImageLoop_Draw(previewType);
	}
}

void Framework::DisplayImageLoop_Update(PreviewType previewType)
{
	// Nothing yet. Can be used to smooth movement of the image previews
}

void Framework::DisplayImageLoop_Draw(PreviewType previewType)
{
	// Clear the color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Single image preview
	switch (previewType){
	case PREVIEWTYPE_MULTIPLEIMAGES:
		DisplayImageLoop_MultipleImages();
		break;
	case PREVIEWTYPE_COMPARISON:
		DisplayImageLoop_ImageComparison();
		break;
	}

	// Swap the buffer and poll events
	glfwSwapBuffers(previewWindow);
	glfwPollEvents();
}

void Framework::DisplayImageLoop_Stop()
{
	previewWindowRunning = false;
}

void Framework::DisplayImageLoop_MultipleImages()
{
	// Iterate over all images and render them at their specified position
	for (size_t i = 0; i < imagePreviewStates.size(); i++){

		ImagePreviewState* imagePreviewState = &(imagePreviewStates[i]);
		GLuint textureBuffer = textureBuffers[i];

		// Get the location of the image
		float x = imagePreviewState->x * scale;
		float y = imagePreviewState->y * scale;
		float width = imagePreviewState->width * scale;
		float height = imagePreviewState->height * scale;

		// Snap to grid if required
		if (glfwGetKey(previewWindow, GLFW_KEY_SPACE) == GLFW_PRESS){
			x = roundf(x / scale / snapgridIntervalX) * snapgridIntervalX * scale;
			y = roundf(y / scale / snapgridIntervalY) * snapgridIntervalY * scale;
		}

		// Calculate position and scale in screen space
		float xPos = (x / ((float)previewWindowWidth / 2.0f)) - 1.0f;
		float yPos = (y / ((float)previewWindowHeight / 2.0f)) - 1.0f;
		float scaleX = (2.0f / (float)previewWindowWidth);
		float scaleY = (2.0f / (float)previewWindowHeight);

		glBindBuffer(GL_ARRAY_BUFFER, quadBuffer);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureBuffer);
		GLuint textureSamplerID = glGetUniformLocation(shaderProgram, "texDiffuse");
		glUniform1i(textureSamplerID, 0);

		// Generate the coordinates
		GLfloat box[4][4] = {
			{ xPos, -yPos, 0, 0 },
			{ xPos + (width * scaleX), -yPos, 1, 0 },
			{ xPos, -yPos - (height * scaleY), 0, 1 },
			{ xPos + (width * scaleX), -yPos - (height * scaleY), 1, 1 }
		};

		// Draw the image
		glBufferData(GL_ARRAY_BUFFER, sizeof(box), box, GL_DYNAMIC_DRAW);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
}

void Framework::DisplayImageLoop_ImageComparison()
{
	// Note: imagePreviewState[0] is used for both images, to display them on top of eachother at all times
	float x = imagePreviewStates[0].x * scale;
	float y = imagePreviewStates[0].y * scale;
	float width = imagePreviewStates[0].width * scale;
	float height = imagePreviewStates[0].height * scale;

	// Snap to grid if required
	if (glfwGetKey(previewWindow, GLFW_KEY_SPACE) == GLFW_PRESS){
		x = roundf(x / scale / snapgridIntervalX) * snapgridIntervalX * scale;
		y = roundf(y / scale / snapgridIntervalY) * snapgridIntervalY * scale;
	}

	// Calculate position and scale in screen space
	float xPos = (x / ((float)previewWindowWidth / 2.0f)) - 1.0f;
	float yPos = (y / ((float)previewWindowHeight / 2.0f)) - 1.0f;
	float scaleX = (2.0f / (float)previewWindowWidth);
	float scaleY = (2.0f / (float)previewWindowHeight);

	glBindBuffer(GL_ARRAY_BUFFER, quadBuffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

	////////////////////////////////////////////////////////////////
	// Draw the first image                                       //
	////////////////////////////////////////////////////////////////

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureBuffers[0]);
	GLuint textureSamplerIDOne = glGetUniformLocation(shaderProgram, "texDiffuse");
	glUniform1i(textureSamplerIDOne, 0);

	// Generate the coordinates
	GLfloat boxOne[4][4] = {
		{ xPos + (width * scaleX) * comparisonPercentage, -yPos, comparisonPercentage, 0 },
		{ xPos + (width * scaleX), -yPos, 1, 0 },
		{ xPos + (width * scaleX) * comparisonPercentage, -yPos - (height * scaleY), comparisonPercentage, 1 },
		{ xPos + (width * scaleX), -yPos - (height * scaleY), 1, 1 }
	};

	// Draw the image
	glBufferData(GL_ARRAY_BUFFER, sizeof(boxOne), boxOne, GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	////////////////////////////////////////////////////////////////
	// Draw the second image                                      //
	////////////////////////////////////////////////////////////////

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureBuffers[1]);
	GLuint textureSamplerIDTwo = glGetUniformLocation(shaderProgram, "texDiffuse");
	glUniform1i(textureSamplerIDTwo, 0);

	// Generate the coordinates
	GLfloat boxTwo[4][4] = {
		{ xPos, -yPos, 0, 0 },
		{ xPos + ((width * scaleX) * comparisonPercentage), -yPos, 1 * comparisonPercentage, 0 },
		{ xPos, -yPos - (height * scaleY), 0, 1 },
		{ xPos + ((width * scaleX)  * comparisonPercentage), -yPos - (height * scaleY), 1 * comparisonPercentage, 1 }
	};

	// Draw the image
	glBufferData(GL_ARRAY_BUFFER, sizeof(boxTwo), boxTwo, GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Framework::DisplayList_AddImage(Image* image)
{
	// Reserve an OpenGL texture buffer
	GLuint bufferID;
	glGenTextures(1, &bufferID);

	// Send the image's pixel data to the buffer
	SendImageToBuffer(image, bufferID);

	// Add the image to the display list
	ImagePreviewState imagePreviewState;
	imagePreviewState.x = (float)(imagePreviewStates.size() * 16); // Used to initialize the image preview positions in a cascading arrangement
	imagePreviewState.y = (float)(imagePreviewStates.size() * 16); // Used to initialize the image preview positions in a cascading arrangement
	imagePreviewState.width = (float)(image->GetWidth());
	imagePreviewState.height = (float)(image->GetHeight());

	imagePreviewStates.push_back(imagePreviewState);
	textureBuffers.push_back(bufferID);
}

void Framework::DisplayList_Clear()
{
	// Free all reserved OpenGL texture buffers
	for (size_t i = 0; i < textureBuffers.size(); i++)
	{
		glDeleteTextures(1, &textureBuffers[i]);
	}

	// Clear the display list and image state list
	imagePreviewStates.clear();
	textureBuffers.clear();
}

void Framework::SendImageToBuffer(Image* image, GLuint buffer)
{
	// Bind the buffer
	glBindTexture(GL_TEXTURE_2D, buffer);

	// Set the interpolation parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	// Send the pixelData to the buffer
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image->GetWidth(), image->GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image->GetPixelData());
}

void Framework::ImagePreviewMove_Single(int imagePreviewID, float deltaX, float deltaY)
{
	imagePreviewStates[imagePreviewID].x += deltaX;
	imagePreviewStates[imagePreviewID].y += deltaY;
}

void Framework::ImagePreviewMove_All(float deltaX, float deltaY)
{
	for (size_t i = 0; i < imagePreviewStates.size(); i++)
	{
		imagePreviewStates[i].x += deltaX;
		imagePreviewStates[i].y += deltaY;
	}
}



/****************************************************************/
/* INPUT CALLBACK FUNCTIONS                                     */
/****************************************************************/

// Handles keyboard events
void Framework::GLFWKeyboardEventCallback(GLFWwindow* window, int key, int scancode, int action, int modifiers)
{
	// Close the preview window upon pressing escape
	if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
	{
		instance->DisplayImageLoop_Stop();
	}

	// Snap all image previews to the grid upon releasing the space bar
	if (action == GLFW_RELEASE && key == GLFW_KEY_SPACE)
	{
		for (size_t i = 0; i < instance->imagePreviewStates.size(); i++)
		{
			ImagePreviewState* imagePreviewState = &(instance->imagePreviewStates[i]);
			imagePreviewState->x = roundf(imagePreviewState->x / instance->snapgridIntervalX) * instance->snapgridIntervalX;
			imagePreviewState->y = roundf(imagePreviewState->y / instance->snapgridIntervalY) * instance->snapgridIntervalY;
		}
	}
}

// Handles mouse button events
void Framework::GLFWMouseButtonEventCallback(GLFWwindow* window, int button, int action, int modifiers)
{
	// Select an image by clicking it
	if (action == GLFW_PRESS){

		ImagePreviewState* image;
		float scale = instance->scale;
		float mouseX = instance->mouseXPrevious;
		float mouseY = instance->mouseYPrevious;

		instance->selectedPreviewImage = -1;

		// Find the topmost selected image (iterate through the vector bottom-to-top, as lower image previews are displayed on top of higher ones) 
		for (size_t i = instance->imagePreviewStates.size() - 1; i >= 0; i--){

			image = &(instance->imagePreviewStates[i]);
			if (image->x * scale < mouseX &&
				image->y * scale < mouseY &&
				(image->x + image->width) * scale > mouseX &&
				(image->y + image->height) * scale > mouseY){
				instance->selectedPreviewImage = static_cast<int>(i);
				break; // Stop searching for other selection candidates, as the following ones are all rendered below this image (due to lower index)
			}
		}
	}
}

// Handles mouse position events
void Framework::GLFWMousePositionEventCallback(GLFWwindow* window, double xPos, double yPos)
{
	// Calculate the delta mouse position
	float deltaX = (float)((xPos - instance->mouseXPrevious) / instance->scale);
	float deltaY = (float)((yPos - instance->mouseYPrevious) / instance->scale);

	// Left mouse button moves the selected image
	if (instance->activePreviewType == PREVIEWTYPE_MULTIPLEIMAGES)
	{
		if (glfwGetMouseButton(instance->previewWindow, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS && instance->selectedPreviewImage != -1 && (size_t)instance->selectedPreviewImage < instance->imagePreviewStates.size()){

			instance->ImagePreviewMove_Single(instance->selectedPreviewImage, deltaX, deltaY);

		}
	}

	// Left mouse button moves all images in the comparison preview type
	if (instance->activePreviewType == PREVIEWTYPE_COMPARISON){
		if (glfwGetMouseButton(instance->previewWindow, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS && instance->selectedPreviewImage != -1 && (size_t)instance->selectedPreviewImage < instance->imagePreviewStates.size()){

			instance->ImagePreviewMove_All(deltaX, deltaY);

		}
	}

	// Right mouse button moves the camera, thus moving all images
	if (instance->activePreviewType == PREVIEWTYPE_MULTIPLEIMAGES){
		if (glfwGetMouseButton(instance->previewWindow, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS){

			instance->ImagePreviewMove_All(deltaX, deltaY);

		}
	}

	// Right mouse button changes the comparisonPercentage in the comparison preview type
	if (instance->activePreviewType == PREVIEWTYPE_COMPARISON)
	{
		if (glfwGetMouseButton(instance->previewWindow, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS)
		{

			instance->comparisonPercentage = fmaxf(fminf((((float)xPos / instance->scale) - instance->imagePreviewStates[0].x) / instance->imagePreviewStates[0].width, 1.0f), 0.0f);

		}
	}

	// Update the previously registered mouse position
	instance->mouseXPrevious = (float)xPos;
	instance->mouseYPrevious = (float)yPos;
}

// Handles mouse position events
void Framework::GLFWMouseScrollEventCallback(GLFWwindow* window, double xOffset, double yOffset)
{
	// Find the current center of the view
	float centerX = instance->previewWindowWidth / 2 / instance->scale;
	float centerY = instance->previewWindowHeight / 2 / instance->scale;

	// Scrolling up zooms in
	if (yOffset > 0){
		instance->scale *= 1.1f;
	}

	// Scrolling down zooms out
	if (yOffset < 0){
		instance->scale /= 1.1f;
	}

	// Find the new center of the view
	float centerXNew = instance->previewWindowWidth / 2 / instance->scale;
	float centerYNew = instance->previewWindowHeight / 2 / instance->scale;

	// Move all images so that they remain in the same position with respect to the center of the view
	instance->ImagePreviewMove_All(-(centerX - centerXNew), -(centerY - centerYNew));
}



/****************************************************************/
/* CONSTRUCTORS AND DESTRUCTORS                                 */
/****************************************************************/

// Default constructor
Framework::Framework()
{
	// Initialize the Framework, print to the console if an error occurs
	if (!Initialize()){
		std::cout << "[Framework] An error occured while initializing the Framework! Check if any files are missing from the executable folder (eg. DLL files)." << std::endl;
	}
}

// Default destructor
Framework::~Framework()
{
	// Initialize the Framework, print to the console if an error occurs
	if (!Terminate()){
		std::cout << "[Framework] An error occured while terminating the Framework! This may have caused a few memory leaks, but the application is shutting down anyways (aka. pretend you didn't see this). " << std::endl;
	}
}



/****************************************************************/
/* IMAGE MANAGEMENT                                             */
/****************************************************************/

// Loads an image and returns it as an Image object
Image* Framework::LoadImageFromFile(std::string filename)
{
	Image* image = NULL;

	// Load the image file
	FREE_IMAGE_FORMAT imageFormat = FreeImage_GetFileType(filename.c_str(), 0);
	if (imageFormat != FIF_UNKNOWN)
	{
		std::cout << "[Framework] Loading an image from file <" << filename << "> " << std::endl;
		FIBITMAP* imageBitmap = FreeImage_Load(imageFormat, filename.c_str());
		FIBITMAP* imageBitmap32Bit = FreeImage_ConvertTo32Bits(imageBitmap); // Convert the image to a 32 bpp (bit-per-pixel) representation. This is in BGRA interleaving.
		FreeImage_FlipVertical(imageBitmap32Bit); // Flip the image vertically to make the positive y-axis point downward

		unsigned char* pixelDataBGRA = (unsigned char*)FreeImage_GetBits(imageBitmap32Bit);
		int width = FreeImage_GetWidth(imageBitmap32Bit);
		int height = FreeImage_GetHeight(imageBitmap32Bit);

		// Convert to RGBA interleaving and store in image (FreeImage stores pixeldata with a BGRA interleaving) 
		image = new Image(width, height);
		unsigned char* pixelData = image->GetPixelData();

		for (int i = 0; i < width * height; i++){
			pixelData[i * 4 + 0] = pixelDataBGRA[i * 4 + 2]; // R value
			pixelData[i * 4 + 1] = pixelDataBGRA[i * 4 + 1]; // G value
			pixelData[i * 4 + 2] = pixelDataBGRA[i * 4 + 0]; // B value
			pixelData[i * 4 + 3] = pixelDataBGRA[i * 4 + 3]; // A value
		}

		// Free memory allocated for FreeImage bitmaps
		FreeImage_Unload(imageBitmap32Bit);
		FreeImage_Unload(imageBitmap);
	}
	else
	{
		std::cerr << "[Framework] Loading an image from file <" << filename << "> failed. Image not found?" << std::endl;
	}

	return image;
}

// Saves an Image object to a bitmap file
void Framework::SaveImageToFile(Image* image, std::string filename)
{
	std::cout << "[Framework] Writing an image to file <" << filename << "> " << std::endl;

	// Convert the image to a FreeImage bitmap
	FIBITMAP* imageBitmap = FreeImage_Allocate(image->GetWidth(), image->GetHeight(), 32);
	RGBQUAD pixelColor;

	char* pixelData = (char*)image->GetPixelData();
	unsigned int width = image->GetWidth();
	unsigned int numPixels = image->GetWidth() * image->GetHeight();

	for (unsigned int i = 0; i < numPixels; i++)
	{
		pixelColor.rgbRed = pixelData[i * 4 + 0];
		pixelColor.rgbGreen = pixelData[i * 4 + 1];
		pixelColor.rgbBlue = pixelData[i * 4 + 2];
		pixelColor.rgbReserved = pixelData[i * 4 + 3];
		FreeImage_SetPixelColor(imageBitmap, i % width, i / width, &pixelColor);
	}

	FreeImage_FlipVertical(imageBitmap); // Flip the image vertically to make the positive y-axis point downward

	// Get the destination file type
	std::string fileExtension = filename.substr(filename.find_last_of(".") + 1);

	FREE_IMAGE_FORMAT fileFormat = FIF_UNKNOWN;
	if (fileExtension.compare("bmp") == 0) fileFormat = FIF_BMP;
	if (fileExtension.compare("png") == 0) fileFormat = FIF_PNG;
	// if (fileExtension.compare("jpg") == 0) fileFormat = FIF_JPEG; // Not working
	// if (fileExtension.compare("jpeg") == 0) fileFormat = FIF_JPEG; // Not working
	// if (fileExtension.compare("gif") == 0) fileFormat = FIF_GIF; // Not working

	// Save the file
	if (!FreeImage_Save(fileFormat, imageBitmap, filename.c_str(), 0)){
		std::cout << "[Framework] Failed to write the image to a file! Check if your file extension is one of the following (in lowercase): bmp, png" << std::endl;
	}
}

void Framework::DisplaySingleImage(Image* image)
{

	// Buffer create the display list for all images
	DisplayList_Clear();

	DisplayList_AddImage(image);

	// Start the display image loop
	DisplayImageLoop(PREVIEWTYPE_MULTIPLEIMAGES);

	DisplayList_Clear();
}

void Framework::DisplayMultipleImages(std::vector<Image*> images)
{
	// Buffer create the display list for all images
	DisplayList_Clear();

	for (size_t i = 0; i < images.size(); i++)
	{
		DisplayList_AddImage(images[i]);
	}

	// Start the display image loop
	DisplayImageLoop(PREVIEWTYPE_MULTIPLEIMAGES);

	DisplayList_Clear();
}

void Framework::DisplayImageComparison(Image* imageOne, Image* imageTwo)
{
	// Buffer create the display list for all images
	DisplayList_Clear();

	DisplayList_AddImage(imageOne);
	DisplayList_AddImage(imageTwo);

	// Start the display image loop
	DisplayImageLoop(PREVIEWTYPE_COMPARISON);

	DisplayList_Clear();
}

