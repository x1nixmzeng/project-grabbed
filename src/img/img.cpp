#include "img.h"

#include "DirectXTex.h"

#include "stb_image_write.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "converters.h"

namespace grabbed
{
    namespace img
    {
        bool makeMetadata(DirectX::TexMetadata& metadata, size_t width, size_t height, XboxD3DFormat sourceFormat)
        {
            metadata.width = width;
            metadata.height = height;
            metadata.depth = 1;
            metadata.arraySize = 1;
            metadata.mipLevels = 1;
            metadata.miscFlags = 0;
            metadata.miscFlags2 = 0;
            metadata.dimension = DirectX::TEX_DIMENSION::TEX_DIMENSION_TEXTURE2D;

            switch (sourceFormat)
            {
            case XboxD3DFormat::X_D3DFMT_DXT1:
                metadata.format = DXGI_FORMAT_BC1_UNORM;
                break;
            case XboxD3DFormat::X_D3DFMT_DXT3:
                metadata.format = DXGI_FORMAT_BC2_UNORM;
                break;
            case XboxD3DFormat::X_D3DFMT_DXT5:
                metadata.format = DXGI_FORMAT_BC3_UNORM;
                break;
            default:
                return false;
            }

            return true;
        }

        bool convertDdsAndSavePng(const buffer& imageData, const string& filename, size_t width, size_t height, XboxD3DFormat sourceFormat)
        {
            DirectX::TexMetadata metadata;

            if (!makeMetadata(metadata, width, height, sourceFormat)) {
                return false;
            }

            bool result = false;
            DirectX::ScratchImage scratch;

            // Note: this is a patched method which allow headerless images with just metadata
            auto hr = DirectX::LoadFromDDSMemoryHeaderless(imageData.data(), imageData.size(), metadata, scratch);
            if (SUCCEEDED(hr)) {
                if (DirectX::IsCompressed(metadata.format)) {
                    auto img = scratch.GetImage(0, 0, 0);

                    // Create a second image for the decompressed copy
                    DirectX::ScratchImage timage;
                    hr = DirectX::Decompress(img, scratch.GetImageCount(), metadata, DXGI_FORMAT_UNKNOWN, timage);

                    if (SUCCEEDED(hr)) {
                        // Assumes images have an alpha
                        auto stbiRes = stbi_write_png(filename.c_str(), metadata.width, metadata.height, 4, timage.GetPixels(), 0);
                        if (stbiRes == 1) {
                            result = true;
                        }
                    }

                    timage.Release();
                }
            }

            scratch.Release();
            return result;
        }

        bool convertRGBAndSavePng(const buffer& imageData, const string& filename, size_t width, size_t height, XboxD3DFormat sourceFormat)
        {
            // We only handle one raw RGB source for now
            if (sourceFormat != XboxD3DFormat::X_D3DFMT_LIN_A8R8G8B8) {
                return false;
            }

            bool result = false;

            // Flip BGRA -> RGBA
            auto pixels = const_cast<u8*>(imageData.data());
            for (size_t i = 0; i < imageData.size(); i += 4) {
                std::swap(pixels[i + 0], pixels[i + 2]);
            }
        
            //  Assumes images have an alpha
            auto stbiRes = stbi_write_png(filename.c_str(), width, height, 4, pixels, 0);
            if (stbiRes == 1) {
                result = true;
            }

            return result;
        }

        bool convertToRGBA(buffer& result, const buffer& imageData, size_t width, size_t height, XboxD3DFormat sourceFormat)
        {
            // 
            return false;
        }

        bool convertToRGBA(buffer& result, const buffer& imageData, size_t width, size_t height, X360TextureFormat sourceFormat)
        {
            if (sourceFormat == X360TextureFormat::DXT1) {

                auto data = ConvertToLinearTexture(imageData, width, height, sourceFormat);

                result = DecodeDXT1(data, width, height);
                
                if (0) {
                    // Assumes images have an alpha
                    auto stbiRes = stbi_write_png("converted_360.png", width, height, 4, result.data(), 0);
                    if (stbiRes == 1) {
                        return true;
                    }
                }

                return true;
            }

            if (sourceFormat == X360TextureFormat::CTX1) {
                auto data = ConvertToLinearTexture(imageData, width, height, sourceFormat);

                result = DecodeCTX1(data, width, height);

                if (1) {
                    // Assumes images have an alpha
                    auto stbiRes = stbi_write_png("ctx.png", width, height, 4, result.data(), 0);
                    if (stbiRes == 1) {
                        return true;
                    }
                }

                return true;
            }

            return false;
        }

        bool convertAndSavePng(const buffer& imageData, const string& filename, size_t width, size_t height, XboxD3DFormat sourceFormat)
        {
            switch (sourceFormat)
            {
            case XboxD3DFormat::X_D3DFMT_DXT1:
            case XboxD3DFormat::X_D3DFMT_DXT3:
            case XboxD3DFormat::X_D3DFMT_DXT5:
                return convertDdsAndSavePng(imageData, filename, width, height, sourceFormat);

            case XboxD3DFormat::X_D3DFMT_LIN_A8R8G8B8:
                return convertRGBAndSavePng(imageData, filename, width, height, sourceFormat);
            }

            return false;
        }

        bool convertAndSavePng(const buffer& imageData, const string& filename, size_t width, size_t height, X360TextureFormat sourceFormat)
        {
            return false;
        }
    }
}
