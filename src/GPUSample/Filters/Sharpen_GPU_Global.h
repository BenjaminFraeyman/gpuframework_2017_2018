#include "RendererAPI.h"
#include "../CommonCL.h"

namespace Filters {
	class Sharpen_GPU_Global : public ImageFilter {
	public:
		Sharpen_GPU_Global(Examples::CLContext& context) : m_Context(context) {}
		Image* Filter(Image* sourceImageOne);
		Examples::CLContext& m_Context;
	};
};