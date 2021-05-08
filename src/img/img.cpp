#include "img.h"

#include "DirectXTex.h"
#include "DDS.h"

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

        buffer createMemoryDds(const buffer& imageData, size_t width, size_t height, X360TextureFormat sourceFormat)
        {
            DirectX::DDS_HEADER imageHeader{};

            imageHeader.size = sizeof(DirectX::DDS_HEADER);
            imageHeader.ddspf.size = sizeof(DirectX::DDS_PIXELFORMAT);
            imageHeader.ddspf.flags = DDS_FOURCC;
            imageHeader.flags = DDS_HEADER_FLAGS_TEXTURE;
            imageHeader.caps = DDS_SURFACE_FLAGS_TEXTURE;

            switch (sourceFormat)
            {
            case X360TextureFormat::DXT1:
                imageHeader.ddspf.fourCC = MAKEFOURCC('D', 'X', 'T', '1');
                break;
            case X360TextureFormat::DXT3:
                imageHeader.ddspf.fourCC = MAKEFOURCC('D', 'X', 'T', '3');
                break;
            case X360TextureFormat::DXT5:
                imageHeader.ddspf.fourCC = MAKEFOURCC('D', 'X', 'T', '5');
                break;
            }

            imageHeader.mipMapCount = 1;

            imageHeader.width = width;
            imageHeader.height = height;

            buffer dds(sizeof(DirectX::DDS_MAGIC) + sizeof(DirectX::DDS_HEADER) + imageData.size());

            auto ptr = dds.data();

            memcpy(ptr, &DirectX::DDS_MAGIC, sizeof(DirectX::DDS_MAGIC));
            ptr += sizeof(DirectX::DDS_MAGIC);
            memcpy(ptr, &imageHeader, sizeof(imageHeader));
            ptr += sizeof(imageHeader);
            memcpy(ptr, imageData.data(), imageData.size());

            return dds;
        }

        bool convertDdsAndSavePng(const buffer& imageData, const std::string& filename, size_t width, size_t height, X360TextureFormat sourceFormat)
        {
            DirectX::TexMetadata metadata;

            bool result = false;
            DirectX::ScratchImage scratch;

            auto data = ConvertToLinearTexture(imageData, width, height, sourceFormat);

            // Make headerless data, headered!
            auto dds{ createMemoryDds(data, width, height, sourceFormat) };

            auto hr = DirectX::LoadFromDDSMemory(dds.data(), dds.size(), DirectX::DDS_FLAGS_NONE, &metadata, scratch);
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

        bool convertRGBAndSavePng(const buffer& imageData, const std::string& filename, size_t width, size_t height, XboxD3DFormat sourceFormat)
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
            switch (sourceFormat)
            {
                case X360TextureFormat::DXT1:
                {
                    auto data = ConvertToLinearTexture(imageData, width, height, sourceFormat);
                    result = DecodeDXT1(data, width, height);
                    return true;
                }

                case X360TextureFormat::DXT3:
                {
                    auto data = ConvertToLinearTexture(imageData, width, height, sourceFormat);
                    result = DecodeDXT3(data, width, height);
                    return true;
                }

                case X360TextureFormat::DXT5:
                {
                    auto data = ConvertToLinearTexture(imageData, width, height, sourceFormat);
                    result = DecodeDXT5(data, width, height);
                    return true;
                }

                case X360TextureFormat::CTX1:
                {
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

                case X360TextureFormat::A8R8G8B8:
                {
                    result = ConvertToLinearTexture(imageData, width, height, sourceFormat);
                    return true;
                }
            }

            return false;
        }

        bool convertAndSavePng(const buffer& imageData, const std::string& filename, size_t width, size_t height, XboxD3DFormat sourceFormat)
        {
            switch (sourceFormat)
            {
            case XboxD3DFormat::X_D3DFMT_DXT1:
            case XboxD3DFormat::X_D3DFMT_DXT3:
            case XboxD3DFormat::X_D3DFMT_DXT5:
                //return convertDdsAndSavePng(imageData, filename, width, height, sourceFormat);
                break;

            case XboxD3DFormat::X_D3DFMT_LIN_A8R8G8B8:
                return convertRGBAndSavePng(imageData, filename, width, height, sourceFormat);
            }

            return false;
        }

        bool convertAndSavePng(const buffer& imageData, const std::string& filename, size_t width, size_t height, X360TextureFormat sourceFormat)
        {
            return false;
        }

        bool savePngFromRGBA(const std::string& filename, size_t width, size_t height, const buffer& imageData)
        {
            return stbi_write_png(filename.c_str(), width, height, 4, imageData.data(), 0);
        }
    }
}
