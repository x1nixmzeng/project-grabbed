#pragma once

#include "base/types.h"
#include "base/texturetypes.h"

namespace grabbed
{
    namespace img
    {
        bool convertToRGBA(buffer& result, const buffer& imageData, size_t width, size_t height, XboxD3DFormat sourceFormat);
        bool convertToRGBA(buffer& result, const buffer& imageData, size_t width, size_t height, X360TextureFormat sourceFormat);

        bool convertAndSavePng(const buffer& imageData, const string& filename, size_t width, size_t height, XboxD3DFormat sourceFormat);
        bool convertAndSavePng(const buffer& imageData, const string& filename, size_t width, size_t height, X360TextureFormat sourceFormat);
    }
}
