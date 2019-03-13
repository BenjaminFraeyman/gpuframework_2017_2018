#include "RendererAPI.h"
#include "../CommonCL.h"

namespace Filters {
	class GB_GPU_Global : public ImageFilter {
	public:
		GB_GPU_Global(Examples::CLContext& context) : m_Context(context) {}
		Image* Filter(Image* sourceImageOne);
		Examples::CLContext& m_Context;
	};
};