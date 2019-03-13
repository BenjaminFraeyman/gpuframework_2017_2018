#ifndef Desaturate_GPU_h
#define Desaturate_GPU_h

#include "RendererAPI.h"
#include "..\CommonCL.h"

// An image filter that applies desaturation to an image by averaging the R, G and B components
namespace Filters {
	class Desaturate_GPU : public ImageFilter {
	public:
		Desaturate_GPU(Examples::CLContext& context) : m_Context(context) {}
		Image* Filter(Image* sourceImageOne);
		Examples::CLContext& m_Context;
	};
};
#endif