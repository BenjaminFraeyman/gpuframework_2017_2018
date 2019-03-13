#ifndef NOISE_GPU
#define NOISE_GPU

#include "RendererAPI.h"
#include "../CommonCL.h"

// An image filter that applies random noise to an image
namespace Filters {
	class Noise_GPU : public ImageFilter{
	public:
		Noise_GPU(Examples::CLContext& context) : m_Context(context) {}
		Image* Filter(Image* sourceImageOne);
		Examples::CLContext& m_Context;
	};
};

#endif