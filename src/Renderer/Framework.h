#ifndef H_Framework
#define H_Framework

#include <GL/glew.h>
//#include <GLFW/glfw3.h>

class Image;
class ImageFilter;

#include "EnumPreviewType.h"
#include "StructImagePreviewState.h"

#include <string> // For text strings
#include <vector> // For holding the states and buffers of an arbitrary number of images

struct GLFWwindow;

// The filter manager contains all functions related to image loading and filtering
class Framework{

public:

	/****************************************************************/
	/* SINGLETON					                                */
	/****************************************************************/

	static Framework* instance;
	static Framework* GetInstance();



private:

	/****************************************************************/
	/* SETTINGS	& PREVIEW WINDOW                                    */
	/****************************************************************/

	int previewWindowAALevel;
	int previewWindowWidth;
	int previewWindowHeight;
	int previewWindowUpdateRate;

	GLFWwindow* previewWindow;
	bool previewWindowRunning;

	std::string vertexShaderFile;
	std::string fragmentShaderFile;



	/****************************************************************/
	/* OPENGL BUFFERS                                               */
	/****************************************************************/

	GLuint shaderProgram;
	std::vector<GLuint> textureBuffers; // Buffers for the individual images
	GLuint quadBuffer; // Buffer for the quads to render images on



	/****************************************************************/
	/* IMAGE PREVIEW STATE                                          */
	/****************************************************************/

	PreviewType activePreviewType;
	float scale;
	float mouseXPrevious;
	float mouseYPrevious;

	float snapgridIntervalX; // X axis intervals to which image previews should snap when grid-dragging
	float snapgridIntervalY; // Y axis intervals to which image previews should snap when grid-dragging

	float comparisonPercentage; // Percentage of the filtered image to show on top of the original image

	int selectedPreviewImage;
	std::vector<ImagePreviewState> imagePreviewStates;



	/****************************************************************/
	/* INITIALIZATION AND TERMINATION                               */
	/****************************************************************/

	// Initializes the filter manager
	bool Initialize();

	// Initializes GLFW
	bool InitializeGLFW();

	// Initializes GLEW
	bool InitializeGLEW();

	// Initializes FreeImage
	bool InitializeFreeImage();

	// Initializes the shader program
	bool InitializeShaderProgram();

	// Initializes OpenCL
	bool InitializeOpenCL();

	// Helper function - Loads and compiles an individual shader
	void LoadAndCompileShader(GLuint shaderID, std::string fileShader);

	void LoadAndCompileShader_DefaultVertexShader(GLuint shaderID);
	void LoadAndCompileShader_DefaultFragmentShader(GLuint shaderID);
	void LoadAndCompileShader_PrintStatus(GLuint shaderID);

	// Initializes OpenGL buffers for image previewing
	bool InitializeBuffers();

	// Registers input callbacks
	bool InitializeInputCallbacks();

	// Terminates the filter manager
	bool Terminate();

	// Terminates GLFW
	bool TerminateGLFW();

	// Terminates GLEW
	bool TerminateGLEW();

	// Terminates FreeImage
	bool TerminateFreeImage();

	// Frees the shader program
	bool TerminateShaderProgram();

	// Frees OpenGL buffers for image previewing
	bool TerminateBuffers();

	// Terminates OpenCL
	bool TerminateOpenCL();



	/****************************************************************/
	/* IMAGE MANAGEMENT                                             */
	/****************************************************************/

	// Rendering loop for the preview window
	void DisplayImageLoop(PreviewType previewType);

	// Updates the state of the preview window
	void DisplayImageLoop_Update(PreviewType previewType);

	// Draws the contents of the preview window
	void DisplayImageLoop_Draw(PreviewType previewType);

	// Stops the rendering loop for the preview window
	void DisplayImageLoop_Stop();

	// Display loop for "MultipleImages" preview mode. Draws multiple draggable images
	void DisplayImageLoop_MultipleImages();

	// Display loop for "ImageComparison" preview mode. Draws two images on top of eachother and allows blending between the two
	void DisplayImageLoop_ImageComparison();

	// Adds the image to the list of images to display
	void DisplayList_AddImage(Image* image);

	// Clears the list of images to display
	void DisplayList_Clear();

	// Sends an image to a buffer for the preview window
	void SendImageToBuffer(Image* image, GLuint buffer);

	// Moves a single image preview
	void ImagePreviewMove_Single(int imagePreviewID, float deltaX, float deltaY);

	// Moves all image previews
	void ImagePreviewMove_All(float deltaX, float deltaY);



	/****************************************************************/
	/* INPUT CALLBACK FUNCTIONS                                     */
	/****************************************************************/

	// Handles keyboard events
	static void GLFWKeyboardEventCallback(GLFWwindow* window, int key, int scancode, int action, int modifiers);

	// Handles mouse button events
	static void GLFWMouseButtonEventCallback(GLFWwindow* window, int button, int action, int modifiers);

	// Handles mouse position events
	static void GLFWMousePositionEventCallback(GLFWwindow* window, double xPos, double yPos);

	// Handles mouse position events
	static void GLFWMouseScrollEventCallback(GLFWwindow* window, double xOffset, double yOffset);



public:

	/****************************************************************/
	/* CONSTRUCTORS AND DESTRUCTORS                                 */
	/****************************************************************/

	// Default constructor
	Framework();

	// Default destructor
	~Framework();



	/****************************************************************/
	/* IMAGE MANAGEMENT                                             */
	/****************************************************************/

	// Loads an image and returns it as an Image object
	Image* LoadImageFromFile(std::string filename);

	// Saves an Image object to a bitmap file
	void SaveImageToFile(Image* image, std::string filename);

	// Displays an image onscreen
	void DisplaySingleImage(Image* image);

	// Displays multiple images onscreen
	void DisplayMultipleImages(std::vector<Image*> images);

	// Displays two images simultaneously for making comparisons
	void DisplayImageComparison(Image* imageOne, Image* imageTwo);
};


// When building with visual studio, this sends all std::cout to the output window

#if defined(_WIN32)
	#if defined(_MSC_VER)
		#include "Windows.h"
		typedef std::basic_streambuf<char, std::char_traits<char>> sbuf_char;
		class vs_stream : sbuf_char
		{
		public:
			vs_stream(sbuf_char *old_buf) : m_OldBuf(old_buf)
			{}
		protected:
			std::streamsize xsputn(const std::char_traits<char>::char_type* s, std::streamsize n)
			{
				m_OldBuf->sputn(s, n);
				OutputDebugString(s);
				return n;
			}

			std::char_traits<char>::int_type overflow(std::char_traits<char>::int_type c)
			{
				char str[2]; str[0] = c; str[1] = 0;
				m_OldBuf->sputn(str, 1);
				OutputDebugString(str);		
				return std::char_traits<char>::int_type(c);
			}

			sbuf_char *m_OldBuf;
		};

		#define HOOK_OUTPUT		sbuf_char *old_buf = std::cout.rdbuf(); sbuf_char *old_err = std::cerr.rdbuf(); vs_stream vs1(old_buf); vs_stream vs2(old_err); std::cout.rdbuf((sbuf_char *)&vs1); std::cerr.rdbuf((sbuf_char *)&vs2);
		#define UNHOOK_OUTPUT	std::cout.rdbuf(old_buf);  std::cerr.rdbuf(old_err); 
	#else
		#define HOOK_OUTPUT	
		#define UNHOOK_OUTPUT
	#endif
#else
	#define HOOK_OUTPUT	
	#define UNHOOK_OUTPUT
#endif

#endif