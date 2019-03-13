#pragma once
#ifndef Invert_GPU_h
#define Invert_GPU_h

#include "RendererAPI.h"
#include "..\CommonCL.h"

namespace Filters {
	class Invert_GPU : public ImageFilter {
	public:
		Invert_GPU(Examples::CLContext& context) : m_Context(context) {}
		Image* Filter(Image* sourceImageOne);
		Examples::CLContext& m_Context;
	};
};

#endif