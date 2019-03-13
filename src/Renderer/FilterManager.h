#ifndef H_FILTERMANAGER
#define H_FILTERMANAGER

class Image;
class ImageFilter;

#include "EnumPreviewType.h"
#include "StructImagePreviewState.h"

#include <string> // For text strings
#include <sstream>
#include <vector> // For holding the states and buffers of an arbitrary number of images
#include <map>

#include "Framework.h"

// The filter manager contains all functions related to image loading and filtering
class ImageFilter;
class FilterSet
{
public:
	FilterSet(std::string name, ImageFilter* cpuImp, ImageFilter* gpuImp)
		: m_CPUFilter(cpuImp), m_GPUFilter(gpuImp), m_Name(name)
	{}
	~FilterSet();
public:
	std::string GetName() const { return m_Name; }
	ImageFilter* GetGPUImp() const { return m_GPUFilter; }
	ImageFilter* GetCPUImp() const { return m_CPUFilter; }

private:
	ImageFilter* m_CPUFilter;
	ImageFilter* m_GPUFilter;

	std::string m_Name;
};

class FilterManager{

public:

	/****************************************************************/
	/* SINGLETON					                                */
	/****************************************************************/

	static FilterManager* GetInstance();

private:
	// Default constructor
	FilterManager();

	static FilterManager* instance;
public:
	virtual ~FilterManager();

	// Copy constructor (unimplemented for singleton pattern)
	FilterManager(FilterManager const&);

	// Assignment operator (unimplemented for singleton pattern)
	FilterManager& operator=(FilterManager const&);
	


public:

	std::string GetOutput() const {	return m_Output.str();	}

	bool RegisterFilter(std::string name, ImageFilter* cpuImp, ImageFilter* gpuImp)
	{
		if (m_Filters.find(name) == m_Filters.end())
		{
			m_Filters[name] = new FilterSet(name, cpuImp, gpuImp);
			return true;
		}
		return false;
	}

	bool ExecuteFilter(std::string name, Image* img);

private:
	std::map<std::string, FilterSet*> m_Filters;
	std::stringstream m_Output;

};

#endif