#include "RendererAPI.h"
#include "../CommonCL.h"

namespace Filters {
	class H_GPU : public ImageFilter {
	public:
		H_GPU(Examples::CLContext& context) : m_Context(context) {}
		Image* Filter(Image* sourceImageOne);
		Examples::CLContext& m_Context;
	};
};