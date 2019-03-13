#include "FilterManager.h"

#include "Image.h"
#include "ImageFilter.h"


#include <iostream> // For printing information to the console
#include <sstream>
#include <fstream> // For reading shader files
#include <ctime> // For mantaining a stable framerate in the update window
#include <vector> // For passing shader compilation errors


FilterSet::~FilterSet()
{
	if (m_CPUFilter) delete m_CPUFilter;
	if (m_GPUFilter) delete m_GPUFilter;
}


/****************************************************************/
/* SINGLETON					                                */
/****************************************************************/

FilterManager* FilterManager::instance = NULL;

FilterManager* FilterManager::GetInstance()
{
	if (!instance){
		instance = new FilterManager();
	}
	return instance;
}

/****************************************************************/
/* CONSTRUCTORS AND DESTRUCTORS                                 */
/****************************************************************/

// Default constructor
FilterManager::FilterManager()
{
}

// Default destructor
FilterManager::~FilterManager()
{
	for (std::map<std::string, FilterSet*>::iterator it = m_Filters.begin(); it != m_Filters.end(); ++it)
	{
		if (it->second)
		{
			delete it->second;
		}
	}
	m_Filters.clear();
}

bool FilterManager::ExecuteFilter(std::string name, Image* img)
{	
	if (m_Filters.find(name) != m_Filters.end())
	{
		FilterSet* fs = m_Filters[name];
		Framework* f = Framework::GetInstance();

		int cpuTime = -1, gpuTime = -1;

		m_Output << "[" << name << "] \t";

		// CPU
		if (fs->GetCPUImp())
		{
			// Execute and time the filter
			time_t tStart = clock();
			Image* convertedImg = fs->GetCPUImp()->Filter(img);
			time_t tDelta = clock() - tStart;

			if (convertedImg && convertedImg != img)
			{
				cpuTime = static_cast<int>(tDelta);
				std::cout << "[" << name << "] \t" << "[CPU] " << cpuTime << "ms" << std::endl;
				f->DisplayImageComparison(img, convertedImg);
				delete convertedImg;
			}
			else
				std::cout << "Problem: ConvertedImage ("<< name <<", CPU) is empty or equal to source image." << std::endl;

		}
		m_Output << "[CPU] " << cpuTime << "ms \t | ";

		// GPU
		if (fs->GetGPUImp())
		{
			// Execute and time the filter
			time_t tStartG = clock();
			Image* convertedImgG = fs->GetGPUImp()->Filter(img);
			time_t tDeltaG = clock() - tStartG;

			if (convertedImgG && convertedImgG != img)
			{
				gpuTime = static_cast<int>(tDeltaG);
				std::cout << "[" << name << "] \t" << "[GPU] " << gpuTime << "ms" << std::endl;
				f->DisplayImageComparison(img, convertedImgG);
				delete convertedImgG;
			}
			else
				std::cout << "Problem: ConvertedImage (" << name <<", GPU) is empty or equal to source image." << std::endl;
			
		}
		m_Output << "[GPU] " << gpuTime << "ms" << std::endl;
		return true;
	}
	return false;
}