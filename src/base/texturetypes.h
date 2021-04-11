#pragma once

#include "types.h"

namespace grabbed
{
    // note; these values are copied from the xdk (d3d8types-xbox.h and d3d8-xbox.h)
    // see bundler source (XDK\Tools\Win32\Graphics\Bundler\)
    enum class XboxD3DFormat : u32
    {
        // Xbox-specific defines, taken from d3d8types-xbox.h and d3d8-xbox.h
        X_D3DFMT_A8R8G8B8 = 0x00000006,
        X_D3DFMT_X8R8G8B8 = 0x00000007,
        X_D3DFMT_R5G6B5 = 0x00000005,
        X_D3DFMT_R6G5B5 = 0x00000027,
        X_D3DFMT_X1R5G5B5 = 0x00000003,
        X_D3DFMT_A1R5G5B5 = 0x00000002,
        X_D3DFMT_A4R4G4B4 = 0x00000004,
        X_D3DFMT_A8 = 0x00000019,
        X_D3DFMT_A8B8G8R8 = 0x0000003A,
        X_D3DFMT_B8G8R8A8 = 0x0000003B,
        X_D3DFMT_R4G4B4A4 = 0x00000039,
        X_D3DFMT_R5G5B5A1 = 0x00000038,
        X_D3DFMT_R8G8B8A8 = 0x0000003C,
        X_D3DFMT_R8B8 = 0x00000029,
        X_D3DFMT_G8B8 = 0x00000028,
        X_D3DFMT_P8 = 0x0000000B,
        X_D3DFMT_L8 = 0x00000000,
        X_D3DFMT_A8L8 = 0x0000001A,
        X_D3DFMT_AL8 = 0x00000001,
        X_D3DFMT_L16 = 0x00000032,
        X_D3DFMT_V8U8 = 0x00000028,
        X_D3DFMT_L6V5U5 = 0x00000027,
        X_D3DFMT_X8L8V8U8 = 0x00000007,
        X_D3DFMT_Q8W8V8U8 = 0x0000003A,
        X_D3DFMT_V16U16 = 0x00000033,
        X_D3DFMT_DXT1 = 0x0000000C, // BC1
        X_D3DFMT_DXT2 = 0x0000000E, // ?
        X_D3DFMT_DXT3 = 0x0000000E, // BC2 Same constant as DXT2
        X_D3DFMT_DXT4 = 0x0000000F, // ?
        X_D3DFMT_DXT5 = 0x0000000F, // BC3 Same constant as DXT4
        X_D3DFMT_LIN_A1R5G5B5 = 0x00000010,
        X_D3DFMT_LIN_A4R4G4B4 = 0x0000001D,
        X_D3DFMT_LIN_A8 = 0x0000001F,
        X_D3DFMT_LIN_A8B8G8R8 = 0x0000003F,
        X_D3DFMT_LIN_A8R8G8B8 = 0x00000012,
        X_D3DFMT_LIN_B8G8R8A8 = 0x00000040,
        X_D3DFMT_LIN_G8B8 = 0x00000017,
        X_D3DFMT_LIN_R4G4B4A4 = 0x0000003E,
        X_D3DFMT_LIN_R5G5B5A1 = 0x0000003D,
        X_D3DFMT_LIN_R5G6B5 = 0x00000011,
        X_D3DFMT_LIN_R6G5B5 = 0x00000037,
        X_D3DFMT_LIN_R8B8 = 0x00000016,
        X_D3DFMT_LIN_R8G8B8A8 = 0x00000041,
        X_D3DFMT_LIN_X1R5G5B5 = 0x0000001C,
        X_D3DFMT_LIN_X8R8G8B8 = 0x0000001E,
        X_D3DFMT_LIN_A8L8 = 0x00000020,
        X_D3DFMT_LIN_AL8 = 0x0000001B,
        X_D3DFMT_LIN_L16 = 0x00000035,
        X_D3DFMT_LIN_L8 = 0x00000013,
        X_D3DFMT_LIN_V16U16 = 0x00000036,
        X_D3DFMT_LIN_V8U8 = 0x00000017,
        X_D3DFMT_LIN_L6V5U5 = 0x00000037,
        X_D3DFMT_LIN_X8L8V8U8 = 0x0000001E,
        X_D3DFMT_LIN_Q8W8V8U8 = 0x00000012,
    };

    // Ported from RareView
    enum class X360TextureFormat : u32
    {
        L8 = 2,
        A8L8 = 0x4a,
        DXT1 = 0x52,
        DXT3 = 0x53,
        DXT5 = 0x54,
        // https://en.wikipedia.org/wiki/3Dc
        DXN = 0x71,

        // cDXN_XY,    // inverted relative to standard ATI2, 360's DXN
        // cDXN_YX     // standard ATI2 eeh

        // Best guess - see https://forum.xentax.com/viewtopic.php?f=10&t=12528
        // http://fileadmin.cs.lth.se/cs/Personal/Michael_Doggett/talks/unc-xenos-doggett.pdf
        CTX1 = 0x7c,

        A8R8G8B8 = 0x86,
    };
}
