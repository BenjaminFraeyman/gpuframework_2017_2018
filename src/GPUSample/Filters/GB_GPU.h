#include "RendererAPI.h"
#include "../CommonCL.h"

namespace Filters {
	class GB_GPU : public ImageFilter {
	public:
		GB_GPU(Examples::CLContext& context) : m_Context(context) {}
		Image* Filter(Image* sourceImageOne);
		Examples::CLContext& m_Context;
	};
};