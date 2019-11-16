#include "textureutils.h"

namespace grabbed
{
    namespace base
    {
        namespace textureutils
        {
            const char* toString(XboxD3DFormat type)
            {
                switch (type)
                {
                case XboxD3DFormat::X_D3DFMT_A8R8G8B8:
                    return "A8R8G8B8";
                case XboxD3DFormat::X_D3DFMT_X8R8G8B8:
                    return "X8R8G8B8 / X8L8V8U8";
                case XboxD3DFormat::X_D3DFMT_R5G6B5:
                    return "R5G6B5";
                case XboxD3DFormat::X_D3DFMT_R6G5B5:
                    return "R6G5B5 / L6V5U5";
                case XboxD3DFormat::X_D3DFMT_X1R5G5B5:
                    return "X1R5G5B5";
                case XboxD3DFormat::X_D3DFMT_A1R5G5B5:
                    return "A1R5G5B5";
                case XboxD3DFormat::X_D3DFMT_A4R4G4B4:
                    return "A4R4G4B4";
                case XboxD3DFormat::X_D3DFMT_A8:
                    return "A8";
                case XboxD3DFormat::X_D3DFMT_A8B8G8R8:
                    return "A8B8G8R8 / Q8W8V8U8";
                case XboxD3DFormat::X_D3DFMT_B8G8R8A8:
                    return "B8G8R8A8";
                case XboxD3DFormat::X_D3DFMT_R4G4B4A4:
                    return "R4G4B4A4";
                case XboxD3DFormat::X_D3DFMT_R5G5B5A1:
                    return "R5G5B5A1";
                case XboxD3DFormat::X_D3DFMT_R8G8B8A8:
                    return "R8G8B8A8";
                case XboxD3DFormat::X_D3DFMT_R8B8:
                    return "R8B8";
                case XboxD3DFormat::X_D3DFMT_G8B8:
                    return "G8B8 / V8U8";
                case XboxD3DFormat::X_D3DFMT_P8:
                    return "P8";
                case XboxD3DFormat::X_D3DFMT_L8:
                    return "L8";
                case XboxD3DFormat::X_D3DFMT_A8L8:
                    return "A8L8";
                case XboxD3DFormat::X_D3DFMT_AL8:
                    return "AL8";
                case XboxD3DFormat::X_D3DFMT_L16:
                    return "L16";
                case XboxD3DFormat::X_D3DFMT_V16U16:
                    return "V16U16";
                case XboxD3DFormat::X_D3DFMT_DXT1:
                    return "BC1 (DXT1)";
                case XboxD3DFormat::X_D3DFMT_DXT3:
                    return "BC2 (DXT3)";
                case XboxD3DFormat::X_D3DFMT_DXT5:
                    return "BC3 (DXT5)";
                case XboxD3DFormat::X_D3DFMT_LIN_A1R5G5B5:
                    return "linear A1R5G5B5";
                case XboxD3DFormat::X_D3DFMT_LIN_A4R4G4B4:
                    return "linear A4R4G4B4";
                case XboxD3DFormat::X_D3DFMT_LIN_A8:
                    return "linear A8";
                case XboxD3DFormat::X_D3DFMT_LIN_A8B8G8R8:
                    return "linear A8B8G8R8";
                case XboxD3DFormat::X_D3DFMT_LIN_A8R8G8B8:
                    return "linear A8R8G8B8 / Q8W8V8U8";
                case XboxD3DFormat::X_D3DFMT_LIN_B8G8R8A8:
                    return "linear B8G8R8A8";
                case XboxD3DFormat::X_D3DFMT_LIN_G8B8:
                    return "linear G8B8 / V8U8";
                case XboxD3DFormat::X_D3DFMT_LIN_R4G4B4A4:
                    return "linear R4G4B4A4";
                case XboxD3DFormat::X_D3DFMT_LIN_R5G5B5A1:
                    return "linear R5G5B5A1";
                case XboxD3DFormat::X_D3DFMT_LIN_R5G6B5:
                    return "linear R5G6B5";
                case XboxD3DFormat::X_D3DFMT_LIN_R6G5B5:
                    return "linear R6G5B5 / L6V5U5";
                case XboxD3DFormat::X_D3DFMT_LIN_R8B8:
                    return "linear R8B8";
                case XboxD3DFormat::X_D3DFMT_LIN_R8G8B8A8:
                    return "linear R8G8B8A8";
                case XboxD3DFormat::X_D3DFMT_LIN_X1R5G5B5:
                    return "linear X1R5G5B5";
                case XboxD3DFormat::X_D3DFMT_LIN_X8R8G8B8:
                    return "linear X8R8G8B8 / X8L8V8U8";
                case XboxD3DFormat::X_D3DFMT_LIN_A8L8:
                    return "linear A8L8";
                case XboxD3DFormat::X_D3DFMT_LIN_AL8:
                    return "linear AL8";
                case XboxD3DFormat::X_D3DFMT_LIN_L16:
                    return "linear L16";
                case XboxD3DFormat::X_D3DFMT_LIN_L8:
                    return "linear L8";

                case XboxD3DFormat::X_D3DFMT_LIN_V16U16:
                    return "linear V16U16";
                default:
                    return "unknown type";
                }
            }

            const char* toString(X360TextureFormat type)
            {
                switch (type)
                {
                case X360TextureFormat::L8:
                    return "L8";
                case X360TextureFormat::A8L8:
                    return "A8L8";
                case X360TextureFormat::DXT1:
                    return "BC1 (DXT1)";
                case X360TextureFormat::DXT3:
                    return "BC2 (DXT3)";
                case X360TextureFormat::DXT5:
                    return "BC3 (DXT5)";
                case X360TextureFormat::DXN:
                    return "DXN/3DC";
                case X360TextureFormat::CTX1:
                    return "CTX1";
                case X360TextureFormat::A8R8G8B8:
                    return "A8R8G8B8";
                default:
                    return "unknown type";
                }
            }
        }
    }
}
