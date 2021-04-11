#pragma once

#include "base/texturetypes.h"

namespace grabbed::base::textureutils
{
    const char* toString(XboxD3DFormat type);
    const char* toString(X360TextureFormat type);
    const char* toString(GenericTextureType type);

    GenericTextureType makeGenericType(XboxD3DFormat originalType);
    GenericTextureType makeGenericType(X360TextureFormat originalType);
}
