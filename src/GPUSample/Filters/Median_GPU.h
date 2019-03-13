#include "RendererAPI.h"
#include "../CommonCL.h"

namespace Filters {
	class Median_GPU : public ImageFilter {
	public:
		Median_GPU(Examples::CLContext& context) : m_Context(context) {}
		Image* Filter(Image* sourceImageOne);
		Examples::CLContext& m_Context;
	};
};