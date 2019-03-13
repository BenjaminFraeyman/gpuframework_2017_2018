#include "RendererAPI.h"
#include "../CommonCL.h"

namespace Filters {
	class ED_GPU : public ImageFilter {
	public:
		ED_GPU(Examples::CLContext& context) : m_Context(context) {}
		Image* Filter(Image* sourceImageOne);
		Examples::CLContext& m_Context;
	};
};