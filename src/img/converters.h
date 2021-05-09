#pragma once

#include "base/types.h"
#include "base/texturetypes.h"

namespace grabbed
{
    namespace img
    {
        size_t GetTextureDataSize(int width, int height, X360TextureFormat textureFormat);

        buffer ConvertToLinearTexture(const buffer& data, int width, int height, X360TextureFormat textureFormat);

        int XGAddress2DTiledX(int Offset, int Width, int TexelPitch);
        int XGAddress2DTiledY(int Offset, int Width, int TexelPitch);

        buffer DecodeDXT1(buffer& data, int width, int height);
        buffer DecodeDXT3(buffer& data, int width, int height);
        buffer DecodeDXT5(buffer& data, int width, int height);

        uint64_t ReadDXNBlockBits(buffer& data, int blockStart);

        // Same as 3Dc normal compression
        // Two-channel version of DXT5A
        // Good for tangent-space normal maps
        buffer DecodeDXN(buffer& data, int width, int height);

        buffer DecodeCTX1(const buffer& data, int width, int height);

    }
}
