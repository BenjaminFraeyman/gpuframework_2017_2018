#include "RendererAPI.h"

// An image filter that applies desaturation to an image by averaging the R, G and B components
namespace Filters {
	class Desaturate_CPU : public ImageFilter{

		Image* Filter(Image* sourceImageOne);

	};
};