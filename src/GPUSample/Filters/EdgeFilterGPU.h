#ifndef GPUPROGRAMMING_EDGEFILTERGPU_H
#define GPUPROGRAMMING_EDGEFILTERGPU_H
#include "RendererAPI.h"
#include "../CommonCL.h"
namespace Filters {
    class EdgeFilterGPU : public ImageFilter{
    public:
        EdgeFilterGPU(Examples::CLContext& context) : m_Context(context) {}
        Image* Filter(Image* sourceImageOne);
        Examples::CLContext& m_Context;
    };
}
#endif //GPUPROGRAMMING_EDGEFILTERGPU_H
