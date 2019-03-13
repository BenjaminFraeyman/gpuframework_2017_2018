#include "RendererAPI.h"
#include "../CommonCL.h"

namespace Filters {
	class Sharpen_GPU : public ImageFilter {
	public:
		Sharpen_GPU(Examples::CLContext& context) : m_Context(context) {}
		Image* Filter(Image* sourceImageOne);
		Examples::CLContext& m_Context;
	};
};