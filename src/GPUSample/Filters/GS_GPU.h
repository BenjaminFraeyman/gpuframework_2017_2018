#ifndef GS_GPU_h
#define GS_GPU_h

#include "RendererAPI.h"
#include "..\CommonCL.h"

namespace Filters {
	class GS_GPU : public ImageFilter {
	public:
		GS_GPU(Examples::CLContext& context) : m_Context(context) {}
		Image* Filter(Image* sourceImageOne);
		Examples::CLContext& m_Context;
	};
};
#endif